/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_H__
#  define __ZERG_H__

#include "utils.h"
#include "zerg/token.h"
#include "zerg/ir.h"
#include "zerg/cfg.h"

#define ZERG_VERSION		"1.0"
#define BUILTIN_LIBRARY		"/usr/local/lib/zerg/__builtin__.zg"

#include <string>
#include <map>
class Zerg : public IR {
	public:
		Zerg(std::string dst, ZergArgs *args);
		virtual ~Zerg();

		void compile(std::string src, ZergArgs *args);
		void emit(std::string op, std::string dst="", std::string src="", std::string idx="", std::string size="");

		std::string regalloc(std::string src, std::string size="");
		void regsave(std::string src);
		void resetreg(void);
		std::string tmpreg(void);
	protected:
		void lexer(std::string src);						/* lexer analysis */
		ZergToken& parser(ZergToken &cur, ZergToken &prev);	/* syntax and semantic analysis */
		void load_namespace(std::map<std::string, VType> &namescope);
		void _load_namespace_(CFG *node, std::map<std::string, VType> &namescope);

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

		/* instance property map */
		std::map<std::string, std::vector<std::string>> _obj_property_;
		std::map<std::string, std::string> _obj_instance_;
};

#endif /* __ZERG_H__ */
