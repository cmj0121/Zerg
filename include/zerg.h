/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_H__
#  define __ZERG_H__

#include <string>
#include <map>

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
	AST_ASSIGN,

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
		void setReg(int nr);
		int  getReg(void);
		void setEmitted(void);
		bool isEmmited(void);

		ASTType type(void);
		std::string data(void);
	private:
		bool _emitted_;
		int _label_, _reg_;
		std::string _raw_;
		ASTType _type_;
};

/* CFG - Control Flow Graph
 *
 * The graph used to describe the control flow (condition and repeat)
 * of the AST, each node means a AST and the edge is the relation between
 * each AST:
 *
 *     1- Always pass-to
 *     2- Boolean case, i.e. True and False
 *
 */
class CFG : public AST {
	public:
		CFG(std::string name);
		virtual ~CFG();

		bool isPassto(void);
		bool isBranch(void);
		void passto(CFG *next);
		void branch(CFG *truecase, CFG *falsecase);
		std::string label(void);

		CFG *nextCFG(bool branch=true);

		friend std::ostream& operator <<(std::ostream &stream, const CFG &src);
	private:
		bool _bypass_, _refed_, _branch_;
		std::string _name_;
		CFG *_next_[2], *_parent_;
#ifdef DEBUG
		/* CFG relation map */
		std::vector<CFG *>stages;
		bool **_map_;

		void buildRelation(CFG *node);
#endif /* DEBUG */
};

class Zerg : public IR {
	public:
		Zerg(std::string dst, off_t entry = 0x1000);
		virtual ~Zerg();

		void compile(std::string src, bool only_ir=false);
		void emit(std::string op, std::string dst="", std::string src="", std::string extra="");

		std::string regalloc(std::string src);
	private:
		bool _only_ir_;
		int _labelcnt_;
		CFG *_root_;
		std::vector<std::pair<std::string, std::string>> _symb_;

		void lexer(void);		/* lexer analysis */
		void parser(void);		/* syntax and semantic analysis */

		void DFS(CFG *node);
		void DFS(AST *node);
		void compileIR(AST *node);
};

#endif /* __ZERG_H__ */
