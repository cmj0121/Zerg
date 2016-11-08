/* Copyright (C) 2016-2016 cmj. All right reserved. */

#include "zerg.h"

static const std::map<std::string, std::pair<ASTType, int>> _map_ = {
	#define DEF(key, value, weight)	{key, {value, weight}}
		DEF("*",	AST_MUL,		2),
		DEF("/",	AST_DIV,		2),
		DEF("%",	AST_MOD,		2),
		DEF("~",	AST_LIKE,		2),

		DEF("+",	AST_ADD,		3),
		DEF("-",	AST_SUB,		3),

		DEF("<<",	AST_LSHT,		4),
		DEF(">>",	AST_RSHT,		4),

		DEF("|",	AST_BIT_OR,		5),
		DEF("&",	AST_BIT_AND,	6),
		DEF("^",	AST_BIT_XOR,	7),

		DEF("not",	AST_LOG_NOT,	8),
		DEF("xor",	AST_LOG_XOR,	9),
		DEF("and",	AST_LOG_AND,	10),
		DEF("or",	AST_LOG_OR,		11),

		/* reserved words */
		DEF("=",		AST_ASSIGN,		0),
		DEF("print",	AST_PRINT,		0),
		DEF("syscall",	AST_SYSCALL,	0),
	#undef DEF
};

ZergToken::ZergToken(const char *src) : std::string(src), _weight_(0) {
	this->classify(src);
}
ZergToken::ZergToken(std::string src) : std::string(src), _weight_(0) {
	this->classify(src);
}

void ZergToken::setType(ASTType type) {
	/* reset the type of the token */
	this->_type_ = type;
}
ASTType ZergToken::type(void) {
	/* return the type of the token */
	return this->_type_;
}
int ZergToken::weight(void) {
	/* reply the weight of the node in AST */
	return this->_weight_;
}

void ZergToken::classify(std::string src) {
	switch(src[0]) {
		case '\0':									/* ROOT */
			this->_type_ = AST_ROOT;
			this->_weight_ = 0xFFFF;
			break;
		case '\t':									/* INDENT */
			this->_type_ = AST_INDENT;
			break;
		case '\r':									/* DEDENT */
			this->_type_ = AST_DEDENT;
			break;
		case '\n':									/* NEWLINE */
			this->_type_ = AST_NEWLINE;
			break;
		case '=':									/* ASSIGN */
			this->_type_ = AST_ASSIGN;
			break;
		case ':':									/* COLON */
			this->_type_ = AST_COLON;
			break;
		case ',':									/* COMMA */
			this->_type_ = AST_COMMA;
			break;
		case '(':									/* PARENTHESES OPEN */
			this->_type_ = AST_PARENTHESES_OPEN;
			break;
		case ')':									/* PARENTHESES CLOSE */
			this->_type_ = AST_PARENTHESES_CLOSE;
			break;
		case '\'': case '"':						/* STRING */
			this->_type_ = AST_STRING;
			break;
		case '+': case '-': case '*': case '/':		/* OPERATOR */
		case '%': case '>': case '<': case '~':
			this->_type_ = AST_OPERATORS;
			for (auto it : _map_) {
				if (it.first == src) {
					this->_type_   = it.second.first;
					this->_weight_ = it.second.second;
					break;
				}
			}
			if (AST_OPERATORS == this->_type_) {
				_D(LOG_CRIT, "Not defile operator `%s`", src.c_str());
				break;
			}
			break;
		case '1': case '2': case '3': case '4':		/* NUMBER */
		case '5': case '6': case '7': case '8':
		case '9': case '0':
			this->_type_   = AST_NUMBER;
			this->_weight_ = 1;
			break;
		default:									/* IDENTIFIER */
			this->_type_   = AST_IDENTIFIER;
			this->_weight_ = 1;
			/* force check the identifier is reserved word or not */
			for (auto it : _map_) {
				if (it.first == src) {
					this->_type_   = it.second.first;
					this->_weight_ = it.second.second;
					break;
				}
			}
			break;
	}

	_D(LOG_DEBUG, "classify token [0x%02X] %s", this->_type_, src.c_str());
}
