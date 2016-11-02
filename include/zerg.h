/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_H__
#  define __ZERG_H__

#include <ostream>
#include <string>
#include <map>

#include "utils.h"
#include "zerg/token.h"
#include "zerg/ir.h"
#include "zerg/cfg.h"

class Zerg : public IR {
	public:
		Zerg(std::string dst, off_t entry = 0x1000);
		virtual ~Zerg();

		void compile(std::string src, bool only_ir=false);
		void emit(std::string op, std::string dst="", std::string src="", std::string extra="");

		std::string regalloc(std::string src);
	protected:
		void lexer(std::string src);						/* lexer analysis */
		ZergToken& parser(ZergToken &cur, ZergToken &prev);	/* syntax and semantic analysis */

		void compileCFG(CFG *node);
		void emitIR(AST *node);
	private:
		int _labelcnt_;
		std::map<std::string, CFG *>_root_;

		std::vector<std::pair<std::string, std::string>> _symb_;
};

#endif /* __ZERG_H__ */
