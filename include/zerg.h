/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_H__
#  define __ZERG_H__

#include "utils.h"
#include "zerg/ir.h"

#define ZERG_VERSION		"1.0"
#define BUILTIN_LIBRARY		"/usr/local/lib/zerg/__builtin__.zg"

#define LEXER_INDENT		"\t"
#define LEXER_DEDENT		"\b"

#include <string>
#include <map>

typedef enum _tag_token_type_ {
	ZTYPE_UNKNOWN = 0,			/* initial type of the Zerg token */
	ZTYPE_NEWLINE,
	ZTYPE_INDENT,
	ZTYPE_DEDENT,

	ZTYPE_NUMBER,
	ZTYPE_STRING,
	ZTYPE_IDENTIFIER,

	ZTYPE_DOT,			/* .  */
	ZTYPE_COMMA,		/* ,  */
	ZTYPE_COLON,		/* :  */
	ZTYPE_SEMICOLON,	/* ;  */

	ZTYPE_LASSIGN,	/* =   */
	ZTYPE_RASSIGN,	/* <-  */
	ZTYPE_ADD,		/* +   */
	ZTYPE_SUB,		/* -   */
	ZTYPE_MUL,		/* *   */
	ZTYPE_DIV,		/* /   */
	ZTYPE_MOD,		/* %   */
	ZTYPE_LIKE,		/* ~   */
	ZTYPE_INC,		/* ++  */
	ZTYPE_DEC,		/* --  */
	ZTYPE_POW,		/* **  */
	ZTYPE_RSHT,		/* >>  */
	ZTYPE_LSHT,		/* <<  */

	ZTYPE_BIT_AND,	/* &   */
	ZTYPE_BIT_OR,	/* |   */
	ZTYPE_BIT_XOR,	/* ^   */

	ZTYPE_LOG_AND,	/* and */
	ZTYPE_LOG_OR,	/* or  */
	ZTYPE_LOG_XOR,	/* xor */
	ZTYPE_LOG_NOT,	/* not */

	ZTYPE_CMP_EQ,	/* eq */
	ZTYPE_CMP_LS,	/* <  */
	ZTYPE_CMP_GT,	/* >  */

	ZTYPE_PAIR_BRAKES_OPEN,		/* [  */
	ZTYPE_PAIR_BRAKES_CLOSE,	/* ]  */
	ZTYPE_PAIR_GROUP_OPEN,		/* (  */
	ZTYPE_PAIR_GROUP_CLOSE,		/* )  */
	ZTYPE_PAIR_DICT_OPEN,		/* {  */
	ZTYPE_PAIR_DICT_CLOSE,		/* }  */

	ZTYPE_CMD_INCLUDE,			/* include  */
	ZTYPE_CMD_PRINT,			/* print    */
	ZTYPE_CMD_FUNCTION,			/* func     */
	ZTYPE_CMD_CLASS,			/* class    */
	ZTYPE_CMD_WHILE,			/* while    */
	ZTYPE_CMD_FOR,				/* for      */
	ZTYPE_CMD_IN,				/* in       */
	ZTYPE_CMD_IF,				/* if       */
	ZTYPE_CMD_ELIF,				/* elif     */
	ZTYPE_CMD_ELSE,				/* else     */
	ZTYPE_CMD_NOP,				/* nop      */
	ZTYPE_CMD_CONTINUE,			/* continue */
	ZTYPE_CMD_BREAK,			/* break    */
	ZTYPE_CMD_ASM,				/* asm      */

	ZTYPE_MAX
} ZType;

typedef std::pair<std::string, ZType> ZergToken;

class Zerg : public IR {
	public:
		Zerg(std::string dst, Args &args);
		virtual ~Zerg();

		/* compile the source code and pass to IR */
		void compile(std::string src);

		virtual void parser(std::string srcfile);
		virtual ZergToken lexer(std::fstream &fp);
	private:
		Args _args_;
		int _lineno_;
		std::string _srcfile_;
};

#endif /* __ZERG_H__ */
