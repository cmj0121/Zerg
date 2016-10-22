/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_TOKEN_H__
#	define __ZERG_TOKEN_H__

#include <string>

#define RESERVED_IF			"if"
#define RESERVED_FUNC		"func"
#define RESERVED_SYSCALL	"syscall"

#define CHECK_RESERVED_TYPE(token, type)	\
	do {									\
		if (RESERVED_##type == token) {		\
			this->_type_ = AST_##type;		\
		}									\
	} while (0)

typedef enum _ASTType_ {
	AST_UNKNOWN			= 0x00,
	AST_ROOT,
	AST_NEWLINE,
	AST_INDENT,
	AST_DEDENT,
	AST_FUNCCALL,

	/* basic type */
	AST_NUMBER			= 0x10,
	AST_STRING,
	AST_IDENTIFIER,
	AST_PARENTHESES_OPEN,
	AST_PARENTHESES_CLOSE,

	/* operators */
	AST_OPERATORS			= 0x20,
	AST_COLON,
	AST_COMMA,
	AST_ASSIGN,

	AST_ADD,
	AST_SUB,
	AST_MUL,
	AST_DIV,
	AST_MOD,
	AST_LIKE,
	AST_LSHT,
	AST_RSHT,

	AST_BIT_OR,
	AST_BIT_AND,
	AST_BIT_XOR,
	AST_LOG_OR,
	AST_LOG_AND,
	AST_LOG_XOR,
	AST_LOG_NOT,

	/* reserved words */
	AST_RESERVED		= 0x100,
	AST_IF,
	AST_FUNC,
	AST_SYSCALL,
} ASTType;

/* token used in Zerg for lexer analysis */
class ZergToken : public std::string {
	public:
		ZergToken (const char *src);
		ZergToken (std::string src);

		void setType(ASTType type);
		ASTType type(void);

		void weight(int weight);
		int  weight(void);
	protected:
		void classify(std::string src);
	private:
		int _weight_;
		ASTType _type_;
};
typedef std::vector<ZergToken> TOKENS;

#endif /* __ZERG_TOKEN_H__ */
