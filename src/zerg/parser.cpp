/* Copyright (C) 2017-2017 cmj. All right reserved. */

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "zerg.h"

#define _SYNTAX(_token)								\
	do {											\
		_D(LOG_CRIT, "syntax error on L#%d %s", 	\
						this->_lineno_,				\
						_token.first.c_str());		\
	} while (0)

/* Parse the source file and generate the CFG by following steps
 *
 *     1. Lexer by function `lexer`
 *     2. Parse and generate a big AST
 *     3. Split into several AST and generate a CFG
 */
AST* Zerg::parser(std::string srcfile) {
	std::string line;
	ZergToken token, next;
	AST *node = new AST("[ROOT]", ZTYPE_UNKNOWN), *cur = NULL;

	this->_fp_.open(srcfile);
	if (!this->_fp_.is_open()) {
		/* cannot open the source code */
		_D(LOG_CRIT, "source file `%s` does NOT exist", srcfile.c_str());
	}

	this->_lineno_  = 0;
	this->_srcfile_ = srcfile;

	for (token = this->lexer(); ZTYPE_UNKNOWN != token.second; ) {
		_D(LOG_DEBUG_LEXER, "-> #0x%-4X %s", token.second, token.first.c_str());

		next = this->lexer();
		if (token.second != ZTYPE_NEWLINE) {
			ALERT(NULL == (cur = this->parse_stmt(token, next)));
			node->insert(cur);
		}
		token = next;
	}

	#if defined(DEBUG_AST) || defined(DEBUG)
		if (node) {
			std::cerr << "==== Full AST ====" << std::endl;
			std::cerr << *node << std::endl;
		}
	#endif /* DEBUG_AST */
	return node;
}
AST* Zerg::parse_stmt(ZergToken token, ZergToken &next) {
	AST *node = NULL;

	_D(LOG_DEBUG_PARSER, "parse stmt on %s", token.first.c_str());
	/* Construct the AST repeat */
	switch(token.second) {
		case ZTYPE_CMD_IF:
		/* if statement */
		case ZTYPE_CMD_WHILE:
		/* while statement */
		case ZTYPE_CMD_FOR:
		/* for statement */
		case ZTYPE_CMD_FUNCTION:
		/* function */
		case ZTYPE_CMD_CLASS:
		/* class statement */
		default:
		/* simple statement */
			node = parse_simple_stmt(token, next);
			break;
	}

	#if defined(DEBUG_AST) || defined(DEBUG)
		if (node) {
			std::cerr << "==== statement ====" << std::endl;
			std::cerr << *node << std::endl;
		}
	#endif /* DEBUG_AST */
	return node;
}
AST* Zerg::parse_simple_stmt(ZergToken token, ZergToken &next) {
	AST *node = NULL;

	_D(LOG_DEBUG_PARSER, "simple statement on %s #%d", token.first.c_str(), token.second);

	switch(token.second) {
		case ZTYPE_CMD_NOP:			/* nop statement */
		case ZTYPE_CMD_CONTINUE:	/* continue statement */
		case ZTYPE_CMD_BREAK:		/* break statement */
			node = new AST(token);
			if (ZTYPE_NEWLINE != next.second) _SYNTAX(token);
				break;
		case ZTYPE_CMD_PRINT:
		/* print statement */
			_SYNTAX(token);
			break;
		default:
			node = this->expression(token, next);
			break;
	}

	return node;
}

/* expression */
AST* Zerg::expression(ZergToken token, ZergToken &next) {
	AST *node = NULL, *sub = NULL;

	do {
		_D(LOG_DEBUG_PARSER, "expression %s #%d", token.first.c_str(), token.second);
		switch(token.second) {
			case ZTYPE_LASSIGN: case ZTYPE_RASSIGN:
				_SYNTAX(token);
				break;
			case ZTYPE_COMMA:
				if (NULL == node && NULL == sub) {
					/* comma following with nothing */
					_SYNTAX(token);
				} else if (NULL == node) {
					node = new AST(token);
					node->insert(sub);
				}
				break;
			default:
				sub = this->test_expr(token, next);
				if (NULL != node) node->insert(sub);
				break;
		}

		token = next;
		next  = this->lexer();
	} while (ZTYPE_NEWLINE != token.second);

	if (NULL == node) node = sub;

	return node;
}
AST* Zerg::test_expr(ZergToken token, ZergToken &next) {
	bool blEndParse = false;
	AST *node = NULL, *op = NULL;

	std::map<ZType, int> OPPriority = {
		{ZTYPE_POW,			1},
		{ZTYPE_MUL,			2}, {ZTYPE_DIV, 2}, {ZTYPE_MOD, 2}, {ZTYPE_LIKE, 2},
		{ZTYPE_ADD,			3}, {ZTYPE_SUB, 3},
		{ZTYPE_RSHT,		4}, {ZTYPE_LSHT, 4},
		{ZTYPE_BIT_AND, 	5},
		{ZTYPE_BIT_XOR,		6},
		{ZTYPE_BIT_OR,		7},
		{ZTYPE_CMP_LS,		8}, {ZTYPE_CMP_GT, 8},
		{ZTYPE_CMP_EQ,		9},
		{ZTYPE_LOG_AND,		10},
		{ZTYPE_LOG_XOR,		11},
		{ZTYPE_LOG_OR,		12},
	};


	/* save the expression as suffix in stack */
	do {
		_D(LOG_DEBUG_PARSER, "expression on %s #%d", token.first.c_str(), token.second);

		switch(token.second) {
			case ZTYPE_LOG_OR:
			case ZTYPE_LOG_XOR:
			case ZTYPE_LOG_AND:
			case ZTYPE_CMP_EQ:
			case ZTYPE_CMP_LS: case ZTYPE_CMP_GT:
			case ZTYPE_BIT_OR:
			case ZTYPE_BIT_XOR:
			case ZTYPE_BIT_AND:
			case ZTYPE_RSHT: case ZTYPE_LSHT:
			case ZTYPE_ADD: case ZTYPE_SUB:
			case ZTYPE_MUL: case ZTYPE_DIV: case ZTYPE_MOD: case ZTYPE_LIKE:
				if (0 == stack.size() || NULL != op) {
					/* NOTE - sign with first statement */
					stack.push_back(this->term_expr(token, next));

					if (op) {
						stack.push_back(op);
						op = NULL;
					}
					break;
				}

				if (NULL != op) _SYNTAX(token);
				op = new AST(token);

				while (3 <= stack.size()) {
					AST *cur = stack[stack.size()-1];

					if (OPPriority[token.second] >= OPPriority[cur->type()]) {
						this->merge_arithmetic(stack);
						continue;
					}

					std::iter_swap(stack.end()-2, stack.end()-1);
					break;
				}
				break;
			case ZTYPE_POW:
				if (NULL != op) _SYNTAX(token);
				op = new AST(token);

				if (3 <= stack.size()) std::iter_swap(stack.end()-2, stack.end()-1);
				break;
			default:
				stack.push_back(this->term_expr(token, next));

				if (NULL != op) {
					stack.push_back(op);
					op = NULL;
				}
				break;
		}

		switch(next.second) {
			case ZTYPE_NEWLINE:
			case ZTYPE_COMMA:
				blEndParse = true;
				break;
			default:
				token = next;
				next  = this->lexer();
				break;
		}
	} while (false == blEndParse && ZTYPE_UNKNOWN != token.second);

	node = this->merge_arithmetic_all(stack);

	return node;
}
AST* Zerg::term_expr(ZergToken token, ZergToken &next) {
	AST *node = NULL;

	_D(LOG_DEBUG_PARSER, "term on %s #%d", token.first.c_str(), token.second);
	switch(token.second) {
		case ZTYPE_ADD: case ZTYPE_SUB: case ZTYPE_LIKE: case ZTYPE_LOG_NOT:	/* term */
			node  = new AST(token);
			token = next;
			next  = this->lexer();
			node->insert(this->term_expr(token, next));
			break;
		default:
			node = this->atom_expr(token, next);
			break;
	}

	return node;
}
AST* Zerg::atom_expr(ZergToken token, ZergToken &next) {
	AST *node = NULL;

	_D(LOG_DEBUG_PARSER, "atom on %s", token.first.c_str());
	switch(token.second) {
		case ZTYPE_NUMBER: case ZTYPE_STRING: case ZTYPE_IDENTIFIER: /* atom */
			node = new AST(token);
			break;
		default:
			_SYNTAX(token);
			break;
	}

	return node;
}

/* arithmetic merge utils function */
AST* Zerg::merge_arithmetic(std::vector<AST *> &stack) {
	AST *node = NULL, *left = NULL, *right = NULL;

	ALERT(0 == stack.size() || 2 == stack.size());
	if (3 <= stack.size()) {
		node  = stack[stack.size()-1];
		left  = stack[stack.size()-3],
		right = stack[stack.size()-2];

		stack.pop_back();
		stack.pop_back();
		stack.pop_back();

		node->insert(left);
		node->insert(right);

		stack.push_back(node);
		if (1 < stack.size()) std::iter_swap(stack.end()-1, stack.end()-2);
	} else if (1 == stack.size()) {
		node = stack[0];
		stack.pop_back();
	}
	return node;
}
AST* Zerg::merge_arithmetic_all(std::vector<AST *> &stack) {
	AST *node = NULL;

	while(0 != stack.size()) {
		node = this->merge_arithmetic(stack);
	}

	return node;
}
