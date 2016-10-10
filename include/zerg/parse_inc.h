#ifndef __ZERG_PARSING_TABLE_H__
	#define __ZERG_PARSING_TABLE_H__
/* GRAMMAR rules
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
 * expr           -> log_or_expr 
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
			{ AST_NUMBER,	11},
			{ AST_STRING,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	11},
			{ AST_SUB,	11},
			{ AST_LIKE,	11},
			{ AST_LOG_NOT,	7},
		},
	}, {
		AST_NUMBER, {
			{ AST_NEWLINE,	0},
			{ AST_ADD,	9},
			{ AST_SUB,	9},
			{ AST_MUL,	10},
			{ AST_DIV,	10},
			{ AST_MOD,	10},
			{ AST_LIKE,	10},
			{ AST_LSHT,	8},
			{ AST_RSHT,	8},
			{ AST_BIT_XOR,	8},
			{ AST_LOG_OR,	4},
			{ AST_LOG_AND,	5},
			{ AST_LOG_XOR,	6},
		},
	}, {
		AST_STRING, {
			{ AST_NEWLINE,	0},
			{ AST_ADD,	9},
			{ AST_SUB,	9},
			{ AST_MUL,	10},
			{ AST_DIV,	10},
			{ AST_MOD,	10},
			{ AST_LIKE,	10},
			{ AST_LSHT,	8},
			{ AST_RSHT,	8},
			{ AST_BIT_XOR,	8},
			{ AST_LOG_OR,	4},
			{ AST_LOG_AND,	5},
			{ AST_LOG_XOR,	6},
		},
	}, {
		AST_IDENTIFIER, {
			{ AST_NEWLINE,	0},
			{ AST_ASSIGN,	1},
			{ AST_ADD,	9},
			{ AST_SUB,	9},
			{ AST_MUL,	10},
			{ AST_DIV,	10},
			{ AST_MOD,	10},
			{ AST_LIKE,	10},
			{ AST_LSHT,	8},
			{ AST_RSHT,	8},
			{ AST_BIT_XOR,	8},
			{ AST_LOG_OR,	4},
			{ AST_LOG_AND,	5},
			{ AST_LOG_XOR,	6},
		},
	}, {
		AST_ASSIGN, {
			{ AST_NUMBER,	11},
			{ AST_STRING,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	11},
			{ AST_SUB,	11},
			{ AST_LIKE,	11},
			{ AST_LOG_NOT,	7},
		},
	}, {
		AST_ADD, {
			{ AST_NUMBER,	11},
			{ AST_STRING,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	11},
			{ AST_SUB,	11},
			{ AST_LIKE,	11},
		},
	}, {
		AST_SUB, {
			{ AST_NUMBER,	11},
			{ AST_STRING,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	11},
			{ AST_SUB,	11},
			{ AST_LIKE,	11},
		},
	}, {
		AST_MUL, {
			{ AST_NUMBER,	11},
			{ AST_STRING,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	11},
			{ AST_SUB,	11},
			{ AST_LIKE,	11},
		},
	}, {
		AST_DIV, {
			{ AST_NUMBER,	11},
			{ AST_STRING,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	11},
			{ AST_SUB,	11},
			{ AST_LIKE,	11},
		},
	}, {
		AST_MOD, {
			{ AST_NUMBER,	11},
			{ AST_STRING,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	11},
			{ AST_SUB,	11},
			{ AST_LIKE,	11},
		},
	}, {
		AST_LIKE, {
			{ AST_NUMBER,	11},
			{ AST_STRING,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	11},
			{ AST_SUB,	11},
			{ AST_LIKE,	11},
		},
	}, {
		AST_LSHT, {
			{ AST_NUMBER,	11},
			{ AST_STRING,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	11},
			{ AST_SUB,	11},
			{ AST_LIKE,	11},
		},
	}, {
		AST_RSHT, {
			{ AST_NUMBER,	11},
			{ AST_STRING,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	11},
			{ AST_SUB,	11},
			{ AST_LIKE,	11},
		},
	}, {
		AST_BIT_XOR, {
			{ AST_ROOT,	8},
			{ AST_NEWLINE,	0},
			{ AST_ASSIGN,	8},
			{ AST_LOG_OR,	4},
			{ AST_LOG_AND,	5},
			{ AST_LOG_XOR,	6},
		},
	}, {
		AST_LOG_OR, {
			{ AST_ROOT,	4},
			{ AST_NEWLINE,	0},
			{ AST_ASSIGN,	4},
		},
	}, {
		AST_LOG_AND, {
			{ AST_ROOT,	5},
			{ AST_NEWLINE,	0},
			{ AST_ASSIGN,	5},
			{ AST_LOG_OR,	4},
		},
	}, {
		AST_LOG_XOR, {
			{ AST_ROOT,	6},
			{ AST_NEWLINE,	0},
			{ AST_ASSIGN,	6},
			{ AST_LOG_OR,	4},
			{ AST_LOG_AND,	5},
		},
	}, {
		AST_LOG_NOT, {
			{ AST_ROOT,	7},
			{ AST_NEWLINE,	0},
			{ AST_ASSIGN,	7},
			{ AST_LOG_OR,	4},
			{ AST_LOG_AND,	5},
			{ AST_LOG_XOR,	6},
		},
	}, 
};
#endif /* __ZERG_PARSING_TABLE_H__ */
