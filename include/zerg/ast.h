/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_AST_H__
#	define __ZERG_AST_H__

#include "zerg/tree.h"
#include "zerg/token.h"

typedef enum __VALUE_Type__ {
	VTYPE_UNKNOWN	= 0,
	VTYPE_BOOLEAN,
	VTYPE_INTEGER,
	VTYPE_OBJECT,
} VType;

/* AST - Abstract Syntax Tree
 *
 * root - begin of the statements
 * node - operator or variable
 * leaf - NUMBER, STRING or IDENTIFIER
 */
class AST : public Tree<AST> {
	public:
		AST(ZergToken src="");

		AST* insert(ZergToken dst);
		AST* insert(AST *node);
		void setLabel(int nr);
		void setReg(int nr);
		void setReg(std::string reg);
		int  getReg(void);
		int  weight(void);
		void weight(int src);
		void setEmitted(void);
		bool isEmmited(void);

		ASTType type(void);
		VType   vtype(void);
		VType   vtype(VType src);
		std::string data(void);

		friend bool operator== (const AST &obj, const char *src);
	private:
		bool _emitted_, _syscall_reg_;
		int _label_, _reg_, _weight_;
		ZergToken _raw_;
		ASTType _type_;
		VType   _vtype_;
};
#endif /* __ZERG_AST_H__ */
