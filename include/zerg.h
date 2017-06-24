/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_H__
#  define __ZERG_H__

#include "utils.h"
#include "zerg/tree.h"
#include "zerg/ir.h"
#include "zerg/parser.h"

#define ZERG_VERSION		"1.0"
#define BUILTIN_LIBRARY		"/usr/local/lib/zerg/__builtin__.zg"

#define LEXER_INDENT		"\t"
#define LEXER_DEDENT		"\b"

#include <string>
#include <map>


class Zerg : public IR, public Parser {
	public:
		Zerg(std::string dst, Args &args) : IR(dst, args), _args_(args), _regcnt_(0) {};
		virtual ~Zerg() {};

		/* compile the source code and pass to IR */
		void compile(std::string src);
		void emit(IROP op, std::string dst="", std::string src="", std::string size="");

		AST* parser(std::string srcfile);
		AST* emitIR(AST *node);
		virtual AST* emitIR_atom(AST *node);
		virtual AST* emitIR_arithmetic(AST *node);
		virtual AST* emitIR_assignment(AST *node);
		virtual AST* emitIR_subroutine(AST *node);
	private:
		Args _args_;
		int _regcnt_;
};

#endif /* __ZERG_H__ */
