/* Copyright (C) 2017 cmj<cmj@cmj.tw>. All right reserved. */
#ifndef __ZERG_PARSER__
#  define __ZERG_PARSER__

#include "zerg/ast.h"

class Parser {
	public:
		AST* parser(std::string srcfile);
		ZergToken lexer(void);

		/* statement */
		virtual AST* parse_stmt(ZergToken token, ZergToken &next);
		virtual AST* parse_simple_stmt(ZergToken token, ZergToken &next);
		virtual AST* parse_if_stmt(ZergToken token, ZergToken &next);
		virtual AST* parse_while_stmt(ZergToken token, ZergToken &next);
		virtual AST* parse_for_stmt(ZergToken token, ZergToken &next);
		virtual AST* parse_func_stmt(ZergToken token, ZergToken &next);
		virtual AST* parse_cls_stmt(ZergToken token, ZergToken &next);
		/* scope : NEWLINE INDENT stmt+ DEDENT NEWLINE */
		virtual AST* scope(ZergToken token, ZergToken &next);
		/* varargs : VAR ( ',' VAR )* */
		virtual AST* varargs(ZergToken token, ZergToken &next);

		/* expression */
		virtual AST* expression(ZergToken token, ZergToken &next);
		virtual AST* test_expr(ZergToken token, ZergToken &next);
		virtual AST* term_expr(ZergToken token, ZergToken &next);
		virtual AST* atom_expr(ZergToken token, ZergToken &next);

		/** merge the latest three node in arithmetic scope */
		virtual AST* merge_arithmetic(std::vector<AST *> &stack);
		virtual AST* merge_arithmetic_all(std::vector<AST *> &stack);
	private:
		int _lineno_;
		std::string _srcfile_;
		std::fstream _fp_;
		std::vector<AST *> stack;
};

#endif /* __ZERG_PARSER__ */

