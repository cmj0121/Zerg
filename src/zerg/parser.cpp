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
AST* Parser::parser(std::string srcfile) {
	std::string line;
	ZergToken token, next;
	AST *node = new AST(ZASM_MAIN_FUNCTION, ZTYPE_UNKNOWN), *cur = NULL;

	this->_fp_.open(srcfile);
	if (!this->_fp_.is_open()) {
		/* cannot open the source code */
		_D(LOG_CRIT, "source file `%s` does NOT exist", srcfile.c_str());
	}

	this->_lineno_  = 0;
	this->_srcfile_ = srcfile;

	for (token = this->lexer(); ZTYPE_UNKNOWN != token.second; ) {
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
AST* Parser::parse_stmt(ZergToken token, ZergToken &next) {
	AST *node = NULL;

	_D(LOG_DEBUG_PARSER, "parse stmt on %s", token.first.c_str());
	/* Construct the AST repeat */
	switch(token.second) {
		case ZTYPE_CMD_IF: 			/* if statement */
			node = this->parse_if_stmt(token, next);
			break;
		case ZTYPE_CMD_WHILE: 		/* while statement */
			node = this->parse_while_stmt(token, next);
			break;
		case ZTYPE_CMD_FOR:			/* for statement */
			node = this->parse_for_stmt(token, next);
			break;
		case ZTYPE_CMD_FUNCTION:	/* function */
			node = this->parse_func_stmt(token, next);
			break;
		case ZTYPE_CMD_CLASS:		/* class statement */
			node = this->parse_cls_stmt(token, next);
			break;
		default:
		/* simple statement */
			node = parse_simple_stmt(token, next);
			break;
	}

	#if defined(DEBUG)
		if (node) {
			std::cerr << "==== statement ====" << std::endl;
			std::cerr << *node << std::endl;
		}
	#endif /* DEBUG */
	return node;
}
AST* Parser::parse_simple_stmt(ZergToken token, ZergToken &next) {
	AST *node = NULL, *sub = NULL;

	_D(LOG_DEBUG_PARSER, "simple statement on %s #%d", token.first.c_str(), token.second);
	switch(token.second) {
		case ZTYPE_CMD_ASM:			/* inline asm */
			if (ZTYPE_STRING != next.second) _SYNTAX(token);
			node = new AST(token);
			node->insert(new AST(next));

			token = next;
			next  = this->lexer();
			if (ZTYPE_NEWLINE != next.second) _SYNTAX(next);
			break;
		case ZTYPE_CMD_NOP:			/* nop statement */
		case ZTYPE_CMD_CONTINUE:	/* continue statement */
		case ZTYPE_CMD_BREAK:		/* break statement */
			node = new AST(token);
			if (ZTYPE_NEWLINE != next.second) _SYNTAX(next);
				break;
		case ZTYPE_CMD_RETURN:		/* return statement */
		case ZTYPE_CMD_PRINT:		/* print statement */
			node  = new AST(token);
			token = next;
			next  = this->lexer();
			node->insert(this->expression(token, next));
			break;
		case ZTYPE_IDENTIFIER:		/* special statement : inc / dec */
		default:
			sub = this->expression(token, next);
			switch(next.second) {
				case ZTYPE_NEWLINE:
					node = sub;
					break;
				case ZTYPE_LASSIGN: case ZTYPE_RASSIGN:
					node  = new AST(next);
					token = this->lexer();
					next  = this->lexer();
					node->insert(sub);
					node->insert(this->expression(token, next));
					break;
				default:
					_SYNTAX(next);
					break;
			}
			break;
	}

	return node;
}
AST* Parser::parse_if_stmt(ZergToken token, ZergToken &next) {
	/* if_stmt : 'if' test_expr ':' scope [ 'else' ':' scope ] */
	AST *node = new AST(token, this->_lineno_);

	_D(LOG_DEBUG_PARSER, "if statement on %s #%d", token.first.c_str(), token.second);
	ALERT(ZTYPE_CMD_IF != token.second && ZTYPE_CMD_ELIF != token.second);

	/* get the test expression */
	token = next;
	next  = this->lexer();
	node->insert(this->test_expr(token, next));

	token = next;				/* :       */
	next  = this->lexer();
	if (ZTYPE_COLON != token.second) _SYNTAX(token);

	/* scope */
	token = next;
	next  = this->lexer();
	node->insert(this->scope(token, next));

	if (ZTYPE_CMD_ELIF == next.second) {			/* elif part */
		token = next;			/* elif */
		next  = this->lexer();
		node->insert(this->parse_if_stmt(token, next));
	} else if (ZTYPE_CMD_ELSE == next.second) {		/* else part if possible */
		token = next;			/* else */
		next  = this->lexer();

		token = next;			/* :   */
		next  = this->lexer();
		if (ZTYPE_COLON != token.second) _SYNTAX(token);

		token = next;
		next  = this->lexer();
		node->insert(this->scope(token, next));
	}

	return node;
}
AST* Parser::parse_while_stmt(ZergToken token, ZergToken &next) {
	/* while_stmt : 'while' test ':' scope */
	AST *node = new AST(token, this->_lineno_);

	_D(LOG_DEBUG_PARSER, "while statement on %s #%d", token.first.c_str(), token.second);
	ALERT(ZTYPE_CMD_WHILE != token.second);

	/* get the test expression */
	token = next;
	next  = this->lexer();
	node->insert(this->test_expr(token, next));

	token = next;				/* :       */
	next  = this->lexer();
	if (ZTYPE_COLON != token.second) _SYNTAX(token);

	/* scope */
	token = next;
	next  = this->lexer();
	node->insert(this->scope(token, next));

	return node;
}
AST* Parser::parse_for_stmt(ZergToken token, ZergToken &next) {
	AST *node = NULL;

	ALERT(ZTYPE_CMD_FOR != token.second);

	_D(LOG_DEBUG_PARSER, "for statement on %s", token.first.c_str());
	node  = new AST(token);
	token = next;
	next  = this->lexer();

	node->insert(this->varargs(token, next));
	token = next;			/* 'in' */
	next  = this->lexer();
	if (ZTYPE_CMD_IN != token.second) _SYNTAX(token);

	token = next;
	next  = this->lexer();
	node->insert(this->expression(token, next));

	token = next;			/* ':'  */
	next  = this->lexer();
	if (ZTYPE_COLON != token.second) _SYNTAX(token);

	token = next;
	next  = this->lexer();
	node->insert(this->scope(token, next));
	return node;
}
AST* Parser::parse_func_stmt(ZergToken token, ZergToken &next) {
	/* func_stmt : 'func' VAR '(' [ parameter ] ')' ':' scope */
	AST *node = new AST(token), *sub = new AST("[PARAM]", ZTYPE_UNKNOWN);

	_D(LOG_DEBUG_PARSER, "func statement on %s #%d", token.first.c_str(), token.second);
	ALERT(ZTYPE_CMD_FUNCTION != token.second && ZTYPE_CMD_CLASS != token.second);

	token = next;			/* VAR */
	next  = this->lexer();
	if (ZTYPE_IDENTIFIER != token.second) _SYNTAX(token);

	node->insert(new AST(token));
	token = next;			/* '(' */
	next  = this->lexer();
	if (ZTYPE_PAIR_GROUP_OPEN != token.second) _SYNTAX(token);

	do {
		token = next;
		next  = this->lexer();

		switch(token.second) {
			case ZTYPE_IDENTIFIER:
				sub->insert(this->varargs(token, next));
				break;
			case ZTYPE_PAIR_GROUP_CLOSE:
				node->insert(sub);
				sub = NULL;
				break;
			default:
				/* FIXME - should process the parameter */
				_SYNTAX(token);
				break;
		}
	} while (ZTYPE_PAIR_GROUP_CLOSE != token.second && ZTYPE_UNKNOWN != token.second);
	if (ZTYPE_PAIR_GROUP_CLOSE != token.second) _SYNTAX(token);

	token = next;		/* ':'  */
	next  = this->lexer();
	if (ZTYPE_COLON != token.second) _SYNTAX(token);
	token = next;
	next  = this->lexer();

	node->insert(this->scope(token, next));

	return node;
}
AST* Parser::parse_cls_stmt(ZergToken token, ZergToken &next) {
	/* cls_stmt  : 'cls'  VAR '(' [ args ] ')' ':' scope */
	AST *node = new AST(token), *sub = new AST("[PARAM]", ZTYPE_UNKNOWN);

	_D(LOG_DEBUG_PARSER, "cls statement on %s #%d", token.first.c_str(), token.second);
	ALERT(ZTYPE_CMD_FUNCTION != token.second && ZTYPE_CMD_CLASS != token.second);

	token = next;			/* VAR */
	next  = this->lexer();
	if (ZTYPE_IDENTIFIER != token.second) _SYNTAX(token);

	node->insert(new AST(token));
	token = next;			/* '(' */
	next  = this->lexer();
	if (ZTYPE_PAIR_GROUP_OPEN != token.second) _SYNTAX(token);

	do {
		token = next;
		next  = this->lexer();

		_D(LOG_DEBUG_PARSER, "cls statement process %s", token.first.c_str());
		switch(token.second) {
			case ZTYPE_IDENTIFIER:
				sub->insert(this->varargs(token, next));
				std::cerr << *sub << std::endl;
				break;
			case ZTYPE_PAIR_GROUP_CLOSE:
				node->insert(sub);
				sub = NULL;
				break;
			default:
				_SYNTAX(token);
				break;
		}
	} while (ZTYPE_PAIR_GROUP_CLOSE != token.second && ZTYPE_UNKNOWN != token.second);
	if (ZTYPE_PAIR_GROUP_CLOSE != token.second) _SYNTAX(token);

	_DEBUG();
	token = next;		/* ':'  */
	next  = this->lexer();
	if (ZTYPE_COLON != token.second) _SYNTAX(token);
	token = next;
	next  = this->lexer();

	node->insert(this->scope(token, next));

	return node;
}
AST* Parser::scope(ZergToken token, ZergToken &next) {
	/* scope : NEWLINE INDENT stmt+ DEDENT NEWLINE */
	AST *node = new AST("[ROOT]", ZTYPE_UNKNOWN), *sub = NULL;

	if (ZTYPE_NEWLINE != token.second) _SYNTAX(token);
	token = next;
	next = this->lexer();

	if (ZTYPE_INDENT != token.second) _SYNTAX(token);
	token = next;
	next = this->lexer();

	do {
		_D(LOG_DEBUG_PARSER, "scope on %s #%d", token.first.c_str(), token.second);
		switch(token.second) {
			case ZTYPE_NEWLINE:
				token = next;
				next = this->lexer();
				break;
			case ZTYPE_UNKNOWN:
			case ZTYPE_DEDENT:
				break;
			default:
				sub = this->parse_stmt(token, next);
				node->insert(sub);
				token = next;
				next = this->lexer();
				break;
		}
	} while (ZTYPE_DEDENT != token.second && ZTYPE_UNKNOWN != token.second);

	if (ZTYPE_DEDENT != token.second) _SYNTAX(token);

	return node;
}

AST* Parser::varargs(ZergToken token, ZergToken &next) {
	/* varargs : VAR ( ',' VAR )* */
	bool blEOP = false;
	AST *node = NULL, *sub = NULL;

	ALERT(ZTYPE_IDENTIFIER != token.second);

	do {
		_D(LOG_DEBUG_PARSER, "var args on %s", token.first.c_str());
		switch(token.second) {
			case ZTYPE_IDENTIFIER:
				if (NULL != sub) _SYNTAX(token);
				sub   = new AST(token);
				break;
			case ZTYPE_COMMA:
				if (NULL == node) node = new AST(token);
				if (NULL == sub)  _SYNTAX(token);

				node->insert(sub);
				sub   = NULL;
				break;
			default:
				break;
		}

		switch(next.second) {
			case ZTYPE_IDENTIFIER:
			case ZTYPE_COMMA:
				token = next;
				next  = this->lexer();
				break;
			default:
				if (NULL == node) node = new AST(",", ZTYPE_COMMA);
				if (NULL == sub)  _SYNTAX(token);
				node->insert(sub);
				blEOP = true;
				break;
		}
	} while (false == blEOP);

	return node;
}

/* expression */
AST* Parser::expression(ZergToken token, ZergToken &next) {
	bool blEndParse = false;
	AST *node = NULL, *sub = NULL;

	do {
		_D(LOG_DEBUG_PARSER, "expression %s #%d", token.first.c_str(), token.second);
		switch(token.second) {
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

		switch(next.second) {
			case ZTYPE_COMMA:
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
			case ZTYPE_POW:
			case ZTYPE_NUMBER: case ZTYPE_STRING: case ZTYPE_IDENTIFIER:
				token = next;
				next  = this->lexer();
				break;
			default:
				_D(LOG_DEBUG_PARSER, "exit expression on %s", next.first.c_str());
				blEndParse = true;
				break;
		}
	} while (false == blEndParse && ZTYPE_NEWLINE != token.second);

	if (NULL == node) node = sub;

	return node;
}
AST* Parser::test_expr(ZergToken token, ZergToken &next) {
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


	if (ZTYPE_LOG_NOT == token.second) {
		node  = new AST(token);

		token = next;
		next  = this->lexer();
		node->insert(this->expression(token, next));

		return node;
	}

	/* save the expression as suffix in stack */
	do {
		_D(LOG_DEBUG_PARSER, "test expr on %s #%d", token.first.c_str(), token.second);

		switch(token.second) {
			case ZTYPE_LOG_NOT:
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

				if (ZTYPE_LOG_NOT == token.second) {
					if (ZTYPE_CMP_EQ != next.second) _SYNTAX(token);
					token = next;
					next  = this->lexer();

					op->insert(new AST(token));	/* 'eq'  */
				}

				while (3 <= stack.size()) {
					AST *cur = stack[stack.size()-1];

					if (1 == cur->length() && ZTYPE_LOG_NOT == cur->type()) {
						if (OPPriority[token.second] >= OPPriority[ZTYPE_CMP_EQ]) {
							this->merge_arithmetic(stack);
							continue;
						}
					} else if (OPPriority[token.second] >= OPPriority[cur->type()]) {
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
			case ZTYPE_LOG_NOT:
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
			case ZTYPE_POW:
			case ZTYPE_NUMBER: case ZTYPE_STRING: case ZTYPE_IDENTIFIER:
				token = next;
				next  = this->lexer();
				break;
			default:
				_D(LOG_DEBUG_PARSER, "exit test_expr on %s", next.first.c_str());
				blEndParse = true;
				break;
		}
	} while (false == blEndParse && ZTYPE_UNKNOWN != token.second);

	node = this->merge_arithmetic_all(stack);

	return node;
}
AST* Parser::term_expr(ZergToken token, ZergToken &next) {
	AST *node = NULL;

	_D(LOG_DEBUG_PARSER, "term on %s #%d", token.first.c_str(), token.second);
	switch(token.second) {
		case ZTYPE_INC: case ZTYPE_DEC:
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
AST* Parser::atom_expr(ZergToken token, ZergToken &next) {
	bool blEOP = false;
	AST *node = NULL, *sub = NULL, *cur = NULL, *tmp = NULL;

	_D(LOG_DEBUG_PARSER, "atom on %s", token.first.c_str());
	switch(token.second) {
		case ZTYPE_PAIR_GROUP_OPEN:
			token = next;
			next  = this->lexer();
			node  = this->expression(token, next);
			if (ZTYPE_PAIR_GROUP_CLOSE != next.second) _SYNTAX(next);

			token = next;
			next  = this->lexer();
			break;
		case ZTYPE_PAIR_BRAKES_OPEN:
			token = next;
			next  = this->lexer();
			node  = this->expression(token, next);
			if (ZTYPE_PAIR_BRAKES_CLOSE != next.second) _SYNTAX(next);

			token = next;
			next  = this->lexer();
			break;
		case ZTYPE_PAIR_DICT_OPEN:
			token = next;
			next  = this->lexer();
			node  = this->expression(token, next);
			if (ZTYPE_PAIR_DICT_CLOSE != next.second) _SYNTAX(next);

			token = next;
			next  = this->lexer();
			break;
		case ZTYPE_NONE:
			node = new AST(token);
			node->otype(OBJ_NONE);
			break;
		case ZTYPE_TRUE:
		case ZTYPE_FALSE:
			node = new AST(token);
			node->otype(OBJ_BOOLEAN);
			break;
		case ZTYPE_NUMBER:
			node = new AST(token);
			node->otype(OBJ_INT);
			break;
		case ZTYPE_STRING:
			node = new AST(token);
			node->otype(OBJ_STRING);
			break;
		case ZTYPE_IDENTIFIER: /* atom */
		case ZTYPE_BUILTIN_SYSCALL:
			node = new AST(token);
			break;
		default:
			_SYNTAX(token);
			break;
	}

	cur = node;
	do {
		_D(LOG_DEBUG_PARSER, "special atom check on %s", next.first.c_str());
		switch(next.second) {
			case ZTYPE_DOT:						/* attribute */
				token = next;
				next  = this->lexer();
				sub   = new AST(token);	/* '.' */

				token = next;
				next  = this->lexer();

				sub->insert(node);
				sub->insert(this->atom_expr(token, next));
				node = sub;
				cur  = sub;
				break;
			case ZTYPE_PAIR_GROUP_OPEN:			/* function call */
				token = next;				/* '(' */
				next  = this->lexer();
				sub   = new AST("call", ZTYPE_FUNCCALL);
				sub->insert(cur);

				token = next;
				next  = this->lexer();
				switch(token.second) {
					case ZTYPE_PAIR_GROUP_CLOSE:
						break;
					default:
						cur = this->expression(token, next);
						if (ZTYPE_COMMA != cur->type()) {
							tmp = new AST(",", ZTYPE_COMMA);
							tmp->insert(cur);
							cur = tmp;
						}
						sub->insert(cur);

						token = next;				/* ')' */
						next  = this->lexer();
						if (ZTYPE_PAIR_GROUP_CLOSE != token.second) _SYNTAX(token);
						break;
				}

				cur  = sub;
				node = cur->root();
				break;
			case ZTYPE_PAIR_BRAKES_OPEN:		/* element getter */
				token = next;				/* '[' */
				next  = this->lexer();
				sub   = new AST("getter", ZTYPE_GETTER);
				sub->insert(cur);

				cur   = sub;
				do {
					token = next;
					next  = this->lexer();
					tmp   = this->expression(token, next);

					switch(next.second) {
						case ZTYPE_PAIR_BRAKES_CLOSE:
							cur->insert(tmp);
							blEOP = true;
							break;
						case ZTYPE_COLON:
							if (ZTYPE_COLON != cur->type()) {
								ALERT(NULL == tmp);

								cur = new AST(next);
								cur->insert(tmp);
								sub->insert(cur);
							}
							token = next;
							next  = this->lexer();
							break;
						default:
							_SYNTAX(next);
							break;
					}

				} while (false == blEOP);
				blEOP = false;

				token = next;
				next  = this->lexer();
				cur   = sub;
				node  = cur->root();
				break;
			default:
				blEOP = true;
				break;
		}
	} while (false == blEOP);

	return node;
}

/* arithmetic merge utils function */
AST* Parser::merge_arithmetic(std::vector<AST *> &stack) {
	AST *node = NULL, *left = NULL, *right = NULL;

	ALERT(0 == stack.size() || 2 == stack.size());
	if (3 <= stack.size()) {
		node  = stack[stack.size()-1];
		left  = stack[stack.size()-3],
		right = stack[stack.size()-2];

		stack.pop_back();
		stack.pop_back();
		stack.pop_back();

		if (ZTYPE_LOG_NOT == node->type() && 1 == node->length()) {
			node->child(0)->insert(left);
			node->child(0)->insert(right);
		} else {
			node->insert(left);
			node->insert(right);
		}

		stack.push_back(node);
		if (1 < stack.size()) std::iter_swap(stack.end()-1, stack.end()-2);
	} else if (1 == stack.size()) {
		node = stack[0];
		stack.pop_back();
	}
	return node;
}
AST* Parser::merge_arithmetic_all(std::vector<AST *> &stack) {
	AST *node = NULL;

	while(0 != stack.size()) {
		node = this->merge_arithmetic(stack);
	}

	return node;
}
