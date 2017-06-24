/* Copyright (C) 2017 cmj<cmj@cmj.tw>. All right reserved. */
#ifndef __ZERG_PARSER__
#  define __ZERG_PARSER__

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
	ZTYPE_CMD_ASM,				/* asm      */

	ZTYPE_MAX
} ZType;

typedef std::pair<std::string, ZType> ZergToken;

class AST : public Tree<AST> {
	public:
		AST(std::string token, ZType type) : Tree<AST>(token), _node_(token, type) {};
		AST(ZergToken &token) : Tree<AST>(token.first), _node_(token) {};

		ZType type() { return _node_.second; }
		std::string raw(void) { return this->_node_.first; }
	private:
		ZergToken _node_;
};

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

