/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_H__
#  define __ZERG_H__

#include "utils.h"
#include "zerg/token.h"
#include "zerg/ir.h"
#include "zerg/cfg.h"

#define ZERG_VERSION		"1.0"
#define BUILTIN_LIBRARY		"/usr/local/lib/zerg/__builtin__.zg"

#define LEXER_INDENT		"\t"
#define LEXER_DEDENT		"\b"

#include <string>
#include <map>
class Zerg : public IR {
	public:
		Zerg(std::string dst, Args &args);
		virtual ~Zerg();

		/* compile the source code and pass to IR */
		void compile(std::string src);

		virtual void lexer(std::string srcfile);
		virtual std::string parser(std::string token, std::string prev) { return token; }; 
	private:
		Args _args_;
		int _lineno_;
		std::string _srcfile_;
};

#endif /* __ZERG_H__ */
