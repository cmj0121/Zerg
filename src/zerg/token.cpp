/* Copyright (C) 2016-2016 cmj. All right reserved. */

#include "zerg.h"

ZergToken::ZergToken(const char *src) : std::string(src) {
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
void ZergToken::weight(int weight) {
	/* set the weight in the AST */
	this->_weight_ = weight;
}
int ZergToken::weight(void) {
	/* reply the weight of the node in AST */
	return this->_weight_;
}

void ZergToken::classify(std::string src) {
	switch(src[0]) {
		case '\0':									/* ROOT */
			this->_type_ = AST_ROOT;
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
		case '%': case '>': case '<':
			this->_type_ = AST_OPERATORS;
			/* FIXME - it should be generated by grammar rules */
			if ("+" == src) this->_type_ = AST_ADD;
			if ("-" == src) this->_type_ = AST_SUB;
			if ("*" == src) this->_type_ = AST_MUL;
			if ("/" == src) this->_type_ = AST_DIV;
			if ("%" == src) this->_type_ = AST_MOD;
			break;
		case '1': case '2': case '3': case '4':		/* NUMBER */
		case '5': case '6': case '7': case '8':
		case '9': case '0':
			this->_type_ = AST_NUMBER;
			break;
		default:									/* IDENTIFIER */
			this->_type_ = AST_IDENTIFIER;
			/* force check the identifier is reserved word or not */
			CHECK_RESERVED_TYPE(src, IF);
			CHECK_RESERVED_TYPE(src, FUNC);
			CHECK_RESERVED_TYPE(src, SYSCALL);
			break;
	}

	_D(LOG_DEBUG, "classify token [0x%02X] %s", this->_type_, src.c_str());
}
