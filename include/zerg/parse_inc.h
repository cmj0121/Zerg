#ifndef __ZERG_PARSING_TABLE_H__
	#define __ZERG_PARSING_TABLE_H__
/* GRAMMAR rules
 * 
 * args           -> expr 
 * args           -> expr ',' expr 
 *
 * arith_expr     -> pow 
 * arith_expr     -> pow '+' pow 
 * arith_expr     -> pow '-' pow 
 *
 * assign         -> IDENTIFIER '=' expr 
 *
 * atom           -> NUMBER 
 * atom           -> STRING 
 * atom           -> IDENTIFIER 
 * atom           -> '+' NUMBER 
 * atom           -> '+' STRING 
 * atom           -> '+' IDENTIFIER 
 * atom           -> '-' NUMBER 
 * atom           -> '-' STRING 
 * atom           -> '-' IDENTIFIER 
 * atom           -> '~' NUMBER 
 * atom           -> '~' STRING 
 * atom           -> '~' IDENTIFIER 
 *
 * bit_and_expr   -> bit_xor_expr 
 * bit_and_expr   -> bit_xor_expr '&' bit_and_expr 
 *
 * bit_or_expr    -> bit_and_expr 
 * bit_or_expr    -> bit_and_expr '|' bit_or_expr 
 *
 * bit_xor_expr   -> shift_expr 
 * bit_xor_expr   -> shift_expr '^' bit_xor_expr 
 *
 * expr           -> funccall 
 * expr           -> log_or_expr 
 *
 * funccall       -> IDENTIFIER '(' ')' 
 * funccall       -> IDENTIFIER '(' args ')' 
 * funccall       -> 'syscall' '(' ')' 
 * funccall       -> 'syscall' '(' args ')' 
 *
 * log_and_expr   -> log_xor_expr 
 * log_and_expr   -> log_xor_expr 'and' log_and_expr 
 *
 * log_not_expr   -> bit_xor_expr 
 * log_not_expr   -> 'not' log_not_expr 
 *
 * log_or_expr    -> log_and_expr 
 * log_or_expr    -> log_and_expr 'or' log_or_expr 
 *
 * log_xor_expr   -> log_not_expr 
 * log_xor_expr   -> log_not_expr 'xor' log_xor_expr 
 *
 * pow            -> atom 
 * pow            -> atom '*' atom 
 * pow            -> atom '/' atom 
 * pow            -> atom '%' atom 
 * pow            -> atom '~' atom 
 *
 * shift_expr     -> arith_expr 
 * shift_expr     -> arith_expr '<<' arith_expr 
 * shift_expr     -> arith_expr '>>' arith_expr 
 *
 * stmt           -> NEWLINE 
 * stmt           -> assign NEWLINE 
 * stmt           -> expr NEWLINE 
 *
 */

std::map<ASTType, std::map<ASTType, int>> _table_ = {
	{
		AST_ROOT, {
			{ AST_NEWLINE,	0},
			{ AST_NUMBER,	13},
			{ AST_STRING,	13},
			{ AST_IDENTIFIER,	13},
			{ AST_ADD,	13},
			{ AST_SUB,	13},
			{ AST_LIKE,	13},
			{ AST_LOG_NOT,	9},
			{ AST_SYSCALL,	3},
		},
	}, {
		AST_NUMBER, {
			{ AST_NEWLINE,	0},
			{ AST_PARENTHESES_CLOSE,	3},
			{ AST_COMMA,	4},
			{ AST_ADD,	11},
			{ AST_SUB,	11},
			{ AST_MUL,	12},
			{ AST_DIV,	12},
			{ AST_MOD,	12},
			{ AST_LIKE,	12},
			{ AST_LSHT,	10},
			{ AST_RSHT,	10},
			{ AST_BIT_XOR,	10},
			{ AST_LOG_OR,	6},
			{ AST_LOG_AND,	7},
			{ AST_LOG_XOR,	8},
		},
	}, {
		AST_STRING, {
			{ AST_NEWLINE,	0},
			{ AST_PARENTHESES_CLOSE,	3},
			{ AST_COMMA,	4},
			{ AST_ADD,	11},
			{ AST_SUB,	11},
			{ AST_MUL,	12},
			{ AST_DIV,	12},
			{ AST_MOD,	12},
			{ AST_LIKE,	12},
			{ AST_LSHT,	10},
			{ AST_RSHT,	10},
			{ AST_BIT_XOR,	10},
			{ AST_LOG_OR,	6},
			{ AST_LOG_AND,	7},
			{ AST_LOG_XOR,	8},
		},
	}, {
		AST_IDENTIFIER, {
			{ AST_NEWLINE,	0},
			{ AST_PARENTHESES_OPEN,	3},
			{ AST_PARENTHESES_CLOSE,	3},
			{ AST_COMMA,	4},
			{ AST_ASSIGN,	1},
			{ AST_ADD,	11},
			{ AST_SUB,	11},
			{ AST_MUL,	12},
			{ AST_DIV,	12},
			{ AST_MOD,	12},
			{ AST_LIKE,	12},
			{ AST_LSHT,	10},
			{ AST_RSHT,	10},
			{ AST_BIT_XOR,	10},
			{ AST_LOG_OR,	6},
			{ AST_LOG_AND,	7},
			{ AST_LOG_XOR,	8},
		},
	}, {
		AST_PARENTHESES_OPEN, {
			{ AST_NUMBER,	13},
			{ AST_STRING,	13},
			{ AST_IDENTIFIER,	13},
			{ AST_PARENTHESES_CLOSE,	3},
			{ AST_ADD,	13},
			{ AST_SUB,	13},
			{ AST_LIKE,	13},
			{ AST_LOG_NOT,	9},
			{ AST_SYSCALL,	3},
		},
	}, {
		AST_PARENTHESES_CLOSE, {
			{ AST_NEWLINE,	0},
			{ AST_PARENTHESES_CLOSE,	3},
			{ AST_COMMA,	4},
		},
	}, {
		AST_COMMA, {
			{ AST_NUMBER,	13},
			{ AST_STRING,	13},
			{ AST_IDENTIFIER,	13},
			{ AST_ADD,	13},
			{ AST_SUB,	13},
			{ AST_LIKE,	13},
			{ AST_LOG_NOT,	9},
			{ AST_SYSCALL,	3},
		},
	}, {
		AST_ASSIGN, {
			{ AST_NUMBER,	13},
			{ AST_STRING,	13},
			{ AST_IDENTIFIER,	13},
			{ AST_ADD,	13},
			{ AST_SUB,	13},
			{ AST_LIKE,	13},
			{ AST_LOG_NOT,	9},
			{ AST_SYSCALL,	3},
		},
	}, {
		AST_ADD, {
			{ AST_NUMBER,	13},
			{ AST_STRING,	13},
			{ AST_IDENTIFIER,	13},
			{ AST_ADD,	13},
			{ AST_SUB,	13},
			{ AST_LIKE,	13},
		},
	}, {
		AST_SUB, {
			{ AST_NUMBER,	13},
			{ AST_STRING,	13},
			{ AST_IDENTIFIER,	13},
			{ AST_ADD,	13},
			{ AST_SUB,	13},
			{ AST_LIKE,	13},
		},
	}, {
		AST_MUL, {
			{ AST_NUMBER,	13},
			{ AST_STRING,	13},
			{ AST_IDENTIFIER,	13},
			{ AST_ADD,	13},
			{ AST_SUB,	13},
			{ AST_LIKE,	13},
		},
	}, {
		AST_DIV, {
			{ AST_NUMBER,	13},
			{ AST_STRING,	13},
			{ AST_IDENTIFIER,	13},
			{ AST_ADD,	13},
			{ AST_SUB,	13},
			{ AST_LIKE,	13},
		},
	}, {
		AST_MOD, {
			{ AST_NUMBER,	13},
			{ AST_STRING,	13},
			{ AST_IDENTIFIER,	13},
			{ AST_ADD,	13},
			{ AST_SUB,	13},
			{ AST_LIKE,	13},
		},
	}, {
		AST_LIKE, {
			{ AST_NUMBER,	13},
			{ AST_STRING,	13},
			{ AST_IDENTIFIER,	13},
			{ AST_ADD,	13},
			{ AST_SUB,	13},
			{ AST_LIKE,	13},
		},
	}, {
		AST_LSHT, {
			{ AST_NUMBER,	13},
			{ AST_STRING,	13},
			{ AST_IDENTIFIER,	13},
			{ AST_ADD,	13},
			{ AST_SUB,	13},
			{ AST_LIKE,	13},
		},
	}, {
		AST_RSHT, {
			{ AST_NUMBER,	13},
			{ AST_STRING,	13},
			{ AST_IDENTIFIER,	13},
			{ AST_ADD,	13},
			{ AST_SUB,	13},
			{ AST_LIKE,	13},
		},
	}, {
		AST_BIT_XOR, {
			{ AST_ROOT,	10},
			{ AST_NEWLINE,	0},
			{ AST_PARENTHESES_OPEN,	10},
			{ AST_PARENTHESES_CLOSE,	3},
			{ AST_COMMA,	10},
			{ AST_ASSIGN,	10},
			{ AST_LOG_OR,	6},
			{ AST_LOG_AND,	7},
			{ AST_LOG_XOR,	8},
		},
	}, {
		AST_LOG_OR, {
			{ AST_ROOT,	6},
			{ AST_NEWLINE,	0},
			{ AST_PARENTHESES_OPEN,	6},
			{ AST_PARENTHESES_CLOSE,	3},
			{ AST_COMMA,	6},
			{ AST_ASSIGN,	6},
		},
	}, {
		AST_LOG_AND, {
			{ AST_ROOT,	7},
			{ AST_NEWLINE,	0},
			{ AST_PARENTHESES_OPEN,	7},
			{ AST_PARENTHESES_CLOSE,	3},
			{ AST_COMMA,	7},
			{ AST_ASSIGN,	7},
			{ AST_LOG_OR,	6},
		},
	}, {
		AST_LOG_XOR, {
			{ AST_ROOT,	8},
			{ AST_NEWLINE,	0},
			{ AST_PARENTHESES_OPEN,	8},
			{ AST_PARENTHESES_CLOSE,	3},
			{ AST_COMMA,	8},
			{ AST_ASSIGN,	8},
			{ AST_LOG_OR,	6},
			{ AST_LOG_AND,	7},
		},
	}, {
		AST_LOG_NOT, {
			{ AST_ROOT,	9},
			{ AST_NEWLINE,	0},
			{ AST_PARENTHESES_OPEN,	9},
			{ AST_PARENTHESES_CLOSE,	3},
			{ AST_COMMA,	9},
			{ AST_ASSIGN,	9},
			{ AST_LOG_OR,	6},
			{ AST_LOG_AND,	7},
			{ AST_LOG_XOR,	8},
		},
	}, {
		AST_SYSCALL, {
			{ AST_PARENTHESES_OPEN,	3},
		},
	}, 
};
#endif /* __ZERG_PARSING_TABLE_H__ */
