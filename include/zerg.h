/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_H__
#  define __ZERG_H__

#include <map>

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

typedef struct _struct_zerg_ir_ {
	IROP opcode;
	std::string dst, src, size, index;
} ZergIR;

class Zerg : public IR, public Parser {
	public:
		Zerg(std::string dst, Args &args) : IR(dst, args), _args_(args), _regcnt_(0) {};
		virtual ~Zerg() {};

		/* compile the source code and pass to IR */
		void compile(std::string src);
		AST* parser(AST *node);
		void emit(AST *node, bool init=false);
		void emit(IROP op, STRING dst="", STRING src="", STRING size="", STRING idx="");
		void flush(void);

		AST* emitIR(AST *node);
		virtual AST* emitIR_atom(AST *node);
		virtual AST* emitIR_arithmetic(AST *node);
		virtual AST* emitIR_assignment(AST *node);
		virtual AST* emitIR_subroutine(AST *node);
		virtual AST* emitIR_stmt(AST *node, bool init=false);

		/* built-in function */
		virtual AST *builtin_syscall(AST *node);
		virtual AST *builtin_exit(AST *node);
		virtual AST *builtin_buffer(AST *node);
		virtual AST *builtin_delete(AST *node);
	private:
		Args _args_;
		int _regcnt_;
		std::vector<ZergIR> _ir_stack_;
		std::vector<std::pair<std::string, std::string>> globals_str;
		std::vector<AST *> _subroutine_;
		std::vector<std::string> _loop_label_;
		std::map<std::string, OBJType> _obj_type_map_;
};

#endif /* __ZERG_H__ */
