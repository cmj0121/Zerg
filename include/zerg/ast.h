/* Copyright (C) 2017 cmj<cmj@cmj.tw>. All right reserved. */
#ifndef __ZERG_AST__
#  define __ZERG_AST__

typedef enum _tag_token_type_ {
	ZTYPE_UNKNOWN = 0,			/* initial type of the Zerg token */
	ZTYPE_NEWLINE,
	ZTYPE_INDENT,
	ZTYPE_DEDENT,
	ZTYPE_FUNCCALL,		/* special token */
	ZTYPE_GETTER,

	ZTYPE_NONE,
	ZTYPE_TRUE,
	ZTYPE_FALSE,
	ZTYPE_NUMBER,
	ZTYPE_STRING,
	ZTYPE_IDENTIFIER,

	ZTYPE_DOT,			/* .  */
	ZTYPE_COMMA,		/* ,  */
	ZTYPE_COLON,		/* :  */
	ZTYPE_SEMICOLON,	/* ;  */

	ZTYPE_LASSIGN,	/* =   */
	ZTYPE_RASSIGN,	/* ->  */
	ZTYPE_ADD,		/* +   */
	ZTYPE_SUB,		/* -   */
	ZTYPE_MUL,		/* *   */
	ZTYPE_DIV,		/* /   */
	ZTYPE_MOD,		/* %   */
	ZTYPE_LIKE,		/* ~   */
	ZTYPE_INC,		/* ++  */
	ZTYPE_DEC,		/* --  */
	ZTYPE_POW,		/* **  */
	ZTYPE_RSHT,		/* >>  */
	ZTYPE_LSHT,		/* <<  */

	ZTYPE_BIT_AND,	/* &   */
	ZTYPE_BIT_OR,	/* |   */
	ZTYPE_BIT_XOR,	/* ^   */

	ZTYPE_LOG_AND,	/* and */
	ZTYPE_LOG_OR,	/* or  */
	ZTYPE_LOG_XOR,	/* xor */
	ZTYPE_LOG_NOT,	/* not */

	ZTYPE_CMP_EQ,	/* eq */
	ZTYPE_CMP_LS,	/* <  */
	ZTYPE_CMP_GT,	/* >  */

	ZTYPE_PAIR_BRAKES_OPEN,		/* [  */
	ZTYPE_PAIR_BRAKES_CLOSE,	/* ]  */
	ZTYPE_PAIR_GROUP_OPEN,		/* (  */
	ZTYPE_PAIR_GROUP_CLOSE,		/* )  */
	ZTYPE_PAIR_DICT_OPEN,		/* {  */
	ZTYPE_PAIR_DICT_CLOSE,		/* }  */

	ZTYPE_CMD_INCLUDE,			/* include  */
	ZTYPE_CMD_PRINT,			/* print    */
	ZTYPE_CMD_FUNCTION,			/* func     */
	ZTYPE_CMD_CLASS,			/* class    */
	ZTYPE_CMD_WHILE,			/* while    */
	ZTYPE_CMD_FOR,				/* for      */
	ZTYPE_CMD_IN,				/* in       */
	ZTYPE_CMD_IF,				/* if       */
	ZTYPE_CMD_ELIF,				/* elif     */
	ZTYPE_CMD_ELSE,				/* else     */
	ZTYPE_CMD_NOP,				/* nop      */
	ZTYPE_CMD_CONTINUE,			/* continue */
	ZTYPE_CMD_BREAK,			/* break    */
	ZTYPE_CMD_RETURN,			/* return   */
	ZTYPE_CMD_ASM,				/* asm      */

	ZTYPE_BUILTIN_SYSCALL,		/* syscall  */

	ZTYPE_MAX
} ZType;
typedef std::pair<std::string, ZType> ZergToken;

typedef enum _tag_object_type_ {
	OBJ_UNKNOWN = 0,
	OBJ_NONE,
	OBJ_BOOLEAN,
	OBJ_INT,
	OBJ_FLOAT,
	OBJ_STRING,
	OBJ_OBJECT,
} OBJType;

typedef enum _tag_cfg_type_ {
	CFG_UNKNOWN	= 0x0,
	CFG_BLOCK,
	CFG_BRANCH,
} CFGType;

class AST : public Tree<AST> {
	public:
		AST(AST *node, int lineno=0);
		AST(std::string token, ZType type, int lineno=0);
		AST(ZergToken &token, int lineno=0);

		std::string raw(void) { return this->_node_.first; }
		std::string data(void);
		std::string label(bool condition=true);
		void setReg(int reg) { this->_reg_ = reg; }
		int  getReg(void) { return this->_reg_; }

		void setSymb(std::string src) { this->_symb_ = src; }

		AST* transfer(void)					{ return this->_transfer_; }
		AST* transfer(AST *node);
		AST* branch(bool condition=true)	{ return this->_branch_[condition ? 0 : 1]; }
		AST* branch(AST *node, AST *sub);

		ZType   type()				{ return _node_.second; }
		OBJType otype(void)         { return this->_otype_; }
		void    otype(OBJType type) { this->_otype_ = type; }
		CFGType ctype(void) 		{ return this->_ctype_; }
		void    ctype(CFGType type)	{ this->_ctype_ = type; }
	private:
		ZergToken _node_;
		int _reg_, _lineno_;
		std::string _symb_;
		OBJType _otype_;
		CFGType _ctype_;
		AST *_branch_[2], *_transfer_, *_parent_;
};

#endif /* __ZERG_AST__ */

