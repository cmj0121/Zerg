/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_AST_H__
#	define __ZERG_AST_H__

#include "zerg/tree.h"
#include "zerg/token.h"

/* AST - Abstract Syntax Tree
 *
 * root - begin of the statements
 * node - operator or variable
 * leaf - NUMBER, STRING or IDENTIFIER
 */
class AST : public Tree<AST> {
	public:
		AST(ZergToken src="", ASTType type=AST_UNKNOWN);

		void insert(ZergToken dst, ASTType type=AST_UNKNOWN);
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
		ZergToken _raw_;
		ASTType _type_;
};
#endif /* __ZERG_AST_H__ */
