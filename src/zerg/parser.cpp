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
	ZergToken prev = {"", ZTYPE_NEWLINE}, token;
	AST *node = new AST("[ROOT]", ZTYPE_UNKNOWN), *cur = NULL;

	this->_fp_.open(srcfile);
	if (!this->_fp_.is_open()) {
		/* cannot open the source code */
		_D(LOG_CRIT, "source file `%s` does NOT exist", srcfile.c_str());
	}

	this->_lineno_  = 0;
	this->_srcfile_ = srcfile;

	while ((token = this->lexer()).second != ZTYPE_UNKNOWN) {
		if (prev.second == ZTYPE_NEWLINE && token.second == ZTYPE_NEWLINE) {
			/* Skip continuously NEWLINE */
			continue;
		}

		_D(LOG_DEBUG_LEXER, " -> token - #0x%-4X %s", token.second, token.first.c_str());
		ALERT(NULL == (cur = this->parse_stmt(token)));
		node->insert(cur);
	}

#if defined(DEBUG_AST) || defined(DEBUG)
	if (node) std::cerr << *node << std::endl;
#endif /* DEBUG_AST */
	return node;
}
AST* Zerg::parse_stmt(ZergToken &token) {
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
			node = parse_simple_stmt(token);
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
AST* Zerg::parse_simple_stmt(ZergToken &token) {
	AST *node = NULL;
	ZergToken next, tmp;

	_D(LOG_DEBUG_PARSER, "simple statement on %s #%d", token.first.c_str(), token.second);

	switch(token.second) {
		case ZTYPE_CMD_NOP:			/* nop statement */
		case ZTYPE_CMD_CONTINUE:	/* continue statement */
		case ZTYPE_CMD_BREAK:		/* break statement */
			node = new AST(token);
			next = this->lexer();
			if (ZTYPE_NEWLINE != next.second) _SYNTAX(token);
				break;
		case ZTYPE_CMD_PRINT:
		/* print statement */
			_SYNTAX(token);
			break;
		default:
			node = this->expression(token);
			break;
	}

	return node;
}

AST* Zerg::expression(ZergToken &token) {
	return this->test_expr(token);
}
AST* Zerg::test_expr(ZergToken &token) {
	AST *node = NULL, *op = NULL, *cur = NULL;
	ZergToken prev;
	std::vector<AST *> stack = {};

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


	do {
		_D(LOG_DEBUG_PARSER, "expression on %s", token.first.c_str());
		/* save the expression as suffix in stack */

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
					stack.push_back(this->term_expr(token, prev));
					prev  = token;

					if (op) {
						stack.push_back(op);
						op = NULL;
					}
					break;
				}

				if (NULL != op) _SYNTAX(token);
				op = new AST(token);

				while (3 <= stack.size()) {
					cur = stack[stack.size()-1];

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
				stack.push_back(this->term_expr(token, prev));
				prev  = token;

				if (NULL != op) {
					stack.push_back(op);
					op = NULL;
				}
				break;
		}

		token = this->lexer();
	} while (ZTYPE_NEWLINE != token.second);

	if (NULL != op) _SYNTAX(prev);

	/* transfer the suffix into infix tree */
	while(0 != stack.size()) {
		node = this->merge_arithmetic(stack);
	}
	return node;
}
AST* Zerg::term_expr(ZergToken &token, ZergToken prev) {
	AST *node = NULL;
	ZergToken next;

	_D(LOG_DEBUG_PARSER, "term on %s", token.first.c_str());
	switch(token.second) {
		case ZTYPE_ADD: case ZTYPE_SUB: case ZTYPE_LIKE: case ZTYPE_LOG_NOT:	/* term */
			node = new AST(token);
			next = this->lexer();
			node->insert(this->term_expr(next, token));
			break;
		default:
			node = this->atom_expr(token, prev);
			break;
			break;
	}

	return node;
}
AST* Zerg::atom_expr(ZergToken &token, ZergToken prev) {
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
