/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_TOKEN_H__
#	define __ZERG_TOKEN_H__

#include <string>

typedef enum _ASTType_ {
	AST_UNKNOWN	= 0,
	AST_ROOT,

	/* basic type */
	AST_NUMBER,
	AST_STRING,
	AST_IDENTIFIER,

	/* operators */
	AST_OPERATORS,	/* FIXME */
	AST_ASSIGN,

	/* reserved words */
	AST_INTERRUPT,
	AST_FUNCCALL,
} ASTType;

/* token used in Zerg for lexer analysis */
class ZergToken : public std::string {
	public:
		ZergToken (const char *);
};


#endif /* __ZERG_TOKEN_H__ */
