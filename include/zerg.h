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

#define BUILTIN_LIBRARY		"LIBS/__builtin__.zg"

class Zerg : public IR {
	public:
		Zerg(std::string dst, bool pie = false, off_t entry = 0x1000, bool symb = false);
		virtual ~Zerg();

		void compile(std::string src, bool only_ir = false, bool compile_ir = false);
		void emit(std::string op, std::string dst="", std::string src="", std::string idx="", std::string size="");

		std::string regalloc(std::string src);
		void regsave(std::string src);
		std::string tmpreg(void);
	protected:
		void lexer(std::string src);						/* lexer analysis */
		ZergToken& parser(ZergToken &cur, ZergToken &prev);	/* syntax and semantic analysis */
		void basic_namespace(std::map<std::string, VType> &namescope);

		void compileCFG(CFG *node, std::map<std::string, VType> &namescope);
		void _compileCFG_(CFG *node, std::map<std::string, VType> &namescope);
		void emitIR(AST *node, std::map<std::string, VType> &namescope);
	private:
		int _labelcnt_, _lineno_, _regs_;
		std::string _src_;
		std::map<std::string, CFG *>_root_;

		std::vector<std::pair<std::string, std::string>> _symb_;
		std::vector<std::string> _alloc_regs_ = { USED_REGISTERS };
		std::map<std::string, std::string> _alloc_regs_map_;
};

#endif /* __ZERG_H__ */
