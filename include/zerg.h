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
		Zerg(std::string dst, Args &args) : IR(dst, args), _args_(args) {};
		virtual ~Zerg() {};

		/* compile the source code and pass to IR */
		void compile(std::string src);

		AST* parser(std::string srcfile);
		void emitIR(AST *node);
	private:
		Args _args_;
};

#endif /* __ZERG_H__ */
