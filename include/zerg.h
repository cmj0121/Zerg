/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_H__
#  define __ZERG_H__

#include <ostream>
#include <string>
#include <map>

#include "utils.h"
#include "zerg/ir.h"
#include "zerg/cfg.h"


typedef std::vector<std::string> TOKENS;
class Parser {
	public:
		bool load(std::string src, std::string stmt="stmt");
		bool load(std::string stmt, TOKENS rule, TOKENS front=TOKENS{}, TOKENS ends=TOKENS{});
		bool gentable(std::string stmt, ASTType type=AST_UNKNOWN);

		std::string stmt(int weight);
		int weight(ASTType prev, ASTType cur);

		friend std::ostream& operator <<(std::ostream &stream, const Parser &src);
	private:
		std::string _src_;
		std::vector<std::pair<std::string, std::vector<ASTType>>> _stmt_;
		std::map<std::string, std::vector<ASTType>> _cached_;
		#include "zerg/parse_inc.h"
		std::map<std::string, std::vector<TOKENS>> _rules_;
};

class Zerg : public Parser, public IR {
	public:
		Zerg(std::string dst, off_t entry = 0x1000);
		virtual ~Zerg();

		void compile(std::string src, bool only_ir=false);
		void emit(std::string op, std::string dst="", std::string src="", std::string extra="");

		std::string regalloc(std::string src);
	protected:
		void lexer(void);		/* lexer analysis */
		void parser(void);		/* syntax and semantic analysis */
		AST *parser(std::vector<ZergToken> tokens);

		void compileCFG(CFG *node);
		void DFS(AST *node);
		void emitIR(AST *node);
	private:
		int _labelcnt_;
		std::map<std::string, CFG *>_root_;

		std::vector<std::pair<std::string, std::string>> _symb_;
};

#endif /* __ZERG_H__ */
