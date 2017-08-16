/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_TOKEN_H__
#	define __ZERG_TOKEN_H__

typedef enum _ASTType_ {
	AST_UNKNOWN			= 0x00,
	AST_ROOT,
	AST_NEWLINE,
	AST_INDENT,
	AST_DEDENT,

	/* basic type */
	AST_NUMBER			= 0x10,
	AST_STRING,
	AST_IDENTIFIER,

	/* operators */
	AST_OPERATORS			= 0x20,
	AST_COLON,
	AST_COMMA,
	AST_ASSIGN,
	AST_DOT,

	AST_ADD					= 0x30,
	AST_SUB,
	AST_MUL,
	AST_DIV,
	AST_MOD,
	AST_LIKE,
	AST_LSHT,
	AST_RSHT,

	AST_EQUAL				= 0x40,
	AST_LESS,
	AST_LESS_OR_EQUAL,
	AST_GRATE_OR_EQUAL,
	AST_GRATE,

	AST_BIT_OR				= 0x50,
	AST_BIT_AND,
	AST_BIT_XOR,
	AST_LOG_OR,
	AST_LOG_AND,
	AST_LOG_XOR,
	AST_LOG_NOT,

	AST_INC,
	AST_DEC,

	/* reserved words */
	AST_RESERVED		= 0x100,
	AST_IF,
	AST_ELSE,
	AST_FUNC,
	AST_CLASS,
	AST_RETURN,
	AST_NOP,
	AST_WHILE,
	AST_BREAK,
	AST_CONTINUE,
	AST_DELETE,

	/* built-in function */
	AST_PRINT,
	AST_SYSCALL,

	AST_TRUE,
	AST_FALSE,
	AST_BUILDIN_BUFFER,
	AST_OBJECT,

	/* pair-token */
	AST_PAIR_TOKEN			= 0x200,
	AST_PARENTHESES_OPEN,
	AST_PARENTHESES_CLOSE,
	AST_BRACKET_OPEN,
	AST_BRACKET_CLOSE,
	AST_DICT_OPEN,
	AST_DICT_CLOSE,
} ASTType;

#define IS_OPERATOR(type)	(AST_OPERATORS < type && AST_RESERVED > type)
#define IS_ATOM(node)		(AST_NUMBER == (node)->type() || AST_IDENTIFIER == (node)->type())

typedef struct _tag_ZergArgs_ {
	int  _entry_;
	bool _only_ir_;
	bool _gen_grammar;
	bool _pie_;
	bool _symbol_;
	bool _compile_ir_;
	bool _no_stdlib_;
} ZergArgs;

/* token used in Zerg for lexer analysis */
#include <string>
class ZergToken : public std::string {
	public:
		ZergToken (const char *src="");
		ZergToken (std::string src);

		const char *c_str(void) const;
		void setType(ASTType type);
		ASTType type(void);
		void setIndex(ZergToken *src);

		void weight(int weight);
		int  weight(void);
	protected:
		void classify(std::string src);
	private:
		int _weight_;
		char _buff_[64];
		ASTType _type_;
};

#endif /* __ZERG_TOKEN_H__ */
