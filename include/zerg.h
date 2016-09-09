/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_H__
#  define __ZERG_H__

#include <string>

#include "utils.h"
#include "zerg/ir.h"
#include "zerg/tree.h"


typedef enum _ASTType_ {
	AST_UNKNOWN	= 0,
	AST_ROOT,

	/* basic type */
	AST_NUMBER,
	AST_STRING,
	AST_IDENTIFIER,

	/* operators */
	AST_OPERATORS,	/* FIXME */

	/* reserved words */
	AST_INTERRUPT,
} ASTType;

/* AST - Abstract Syntax Tree
 *
 * root - begin of the statements
 * node - operator or variable
 * leaf - NUMBER, STRING or IDENTIFIER
 */
class AST : public Tree<AST> {
	public:
		AST(std::string src="");

		void insert(std::string dst);
		void setLabel(int nr);

		ASTType type(void);
		std::string data(void);
	private:
		int _label_;
		std::string _raw_;
		ASTType _type_;
};

class Zerg : public IR {
	public:
		Zerg(std::string src, std::string dst, off_t entry = 0x1000);
		virtual ~Zerg();

		void lexer(void);		/* lexer analysis */
		void parser(void);		/* syntax and semantic analysis */

		void DFS(AST *node=NULL);
		void compile(AST *node);
	private:
		int _labelcnt_;
		AST *_root_;
		std::vector<std::pair<std::string, std::string>> _symb_;
};

#endif /* __ZERG_H__ */
