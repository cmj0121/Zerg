#ifndef __ZERG_PARSING_TABLE_H__
	#define __ZERG_PARSING_TABLE_H__
/* GRAMMAR rules
 * 
 * arith_expr -> pow 
 * arith_expr -> pow '+' pow 
 * arith_expr -> pow '-' pow 
 *   assign -> IDENTIFIER '=' expr 
 *     atom -> INT 
 *     atom -> IDENTIFIER 
 *     atom -> '+' atom 
 *     atom -> '-' atom 
 *     atom -> '~' atom 
 * bit_and_expr -> bit_xor_expr 
 * bit_and_expr -> bit_xor_expr '&' bit_xor_expr 
 * bit_or_expr -> bit_and_expr 
 * bit_or_expr -> bit_and_expr '|' bit_and_expr 
 * bit_xor_expr -> shift_expr 
 * bit_xor_expr -> shift_expr '^' shift_expr 
 *     expr -> log_or_expr 
 * log_and_expr -> log_xor_expr 
 * log_and_expr -> log_xor_expr 'and' log_xor_expr 
 * log_not_expr -> bit_xor_expr 
 * log_not_expr -> 'not' log_not_expr 
 * log_or_expr -> log_and_expr 
 * log_or_expr -> log_and_expr 'or' log_and_expr 
 * log_xor_expr -> log_not_expr 
 * log_xor_expr -> log_not_expr 'xor' log_not_expr 
 *      pow -> atom 
 *      pow -> atom '*' atom 
 *      pow -> atom '/' atom 
 *      pow -> atom '%' atom 
 *      pow -> atom '~' atom 
 * shift_expr -> arith_expr 
 * shift_expr -> arith_expr '<<' arith_expr 
 * shift_expr -> arith_expr '>>' arith_expr 
 *     stmt -> NEWLINE 
 *     stmt -> assign NEWLINE 
 *     stmt -> expr NEWLINE 
 */

/* Parsing Table
 *
 *                   NEWLINE           INT               IDENTIFIER        '='               '+'               '-'               '*'               '/'               '%'               '~'               '<<'              '>>'              '|'               '&'               '^'               'or'              'and'             'xor'             'not'             
 * stmt        0           11          11                      12          12                                              12                                                                                                          7           
 * INT         0                                               9           9           10          10          10          10          8           8                                   7           3           4           5                       
 * IDENTIFIER  0                                   1           9           9           10          10          10          10          8           8                                   7           3           4           5                       
 * '='                     11          11                      12          12                                              12                                                                                                          7           
 * '+'         0           11          11          12          9           9           10          10          10          10          8           8                                   7           3           4           5                       
 * '-'         0           11          11          12          9           9           10          10          10          10          8           8                                   7           3           4           5                       
 * '*'                     11          11                      12          12                                              12                                                                                                                      
 * '/'                     11          11                      12          12                                              12                                                                                                                      
 * '%'                     11          11                      12          12                                              12                                                                                                                      
 * '~'         0           11          11          12          9           9           10          10          10          10          8           8                                   7           3           4           5                       
 * '<<'                    11          11                      12          12                                              12                                                                                                                      
 * '>>'                    11          11                      12          12                                              12                                                                                                                      
 * '^'                     11          11                      12          12                                              12                                                                                                                      
 * 'or'                    11          11                      12          12                                              12                                                                                                          7           
 * 'and'                   11          11                      12          12                                              12                                                                                                          7           
 * 'xor'                   11          11                      12          12                                              12                                                                                                          7           
 * 'not'       0                                   7                                                                                                                                               3           4           5                       
 */

std::map<ASTType, std::map<ASTType, int>> _table_ = {
	{
		AST_UNKNOWN, {
			{ AST_NEWLINE,	0},
			{ AST_NUMBER,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	12},
			{ AST_SUB,	12},
			{ AST_LIKE,	12},
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
			{ AST_BIT_XOR,	7},
			{ AST_LOG_OR,	3},
			{ AST_LOG_AND,	4},
			{ AST_LOG_XOR,	5},
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
			{ AST_BIT_XOR,	7},
			{ AST_LOG_OR,	3},
			{ AST_LOG_AND,	4},
			{ AST_LOG_XOR,	5},
		},
	}, {
		AST_ASSIGN, {
			{ AST_NUMBER,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	12},
			{ AST_SUB,	12},
			{ AST_LIKE,	12},
			{ AST_LOG_NOT,	7},
		},
	}, {
		AST_ADD, {
			{ AST_UNKNOWN,	12},
			{ AST_NEWLINE,	0},
			{ AST_NUMBER,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ASSIGN,	12},
			{ AST_ADD,	9},
			{ AST_SUB,	9},
			{ AST_MUL,	10},
			{ AST_DIV,	10},
			{ AST_MOD,	10},
			{ AST_LIKE,	10},
			{ AST_LSHT,	8},
			{ AST_RSHT,	8},
			{ AST_BIT_XOR,	7},
			{ AST_LOG_OR,	3},
			{ AST_LOG_AND,	4},
			{ AST_LOG_XOR,	5},
		},
	}, {
		AST_SUB, {
			{ AST_UNKNOWN,	12},
			{ AST_NEWLINE,	0},
			{ AST_NUMBER,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ASSIGN,	12},
			{ AST_ADD,	9},
			{ AST_SUB,	9},
			{ AST_MUL,	10},
			{ AST_DIV,	10},
			{ AST_MOD,	10},
			{ AST_LIKE,	10},
			{ AST_LSHT,	8},
			{ AST_RSHT,	8},
			{ AST_BIT_XOR,	7},
			{ AST_LOG_OR,	3},
			{ AST_LOG_AND,	4},
			{ AST_LOG_XOR,	5},
		},
	}, {
		AST_MUL, {
			{ AST_NUMBER,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	12},
			{ AST_SUB,	12},
			{ AST_LIKE,	12},
		},
	}, {
		AST_DIV, {
			{ AST_NUMBER,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	12},
			{ AST_SUB,	12},
			{ AST_LIKE,	12},
		},
	}, {
		AST_MOD, {
			{ AST_NUMBER,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	12},
			{ AST_SUB,	12},
			{ AST_LIKE,	12},
		},
	}, {
		AST_LIKE, {
			{ AST_UNKNOWN,	12},
			{ AST_NEWLINE,	0},
			{ AST_NUMBER,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ASSIGN,	12},
			{ AST_ADD,	9},
			{ AST_SUB,	9},
			{ AST_MUL,	10},
			{ AST_DIV,	10},
			{ AST_MOD,	10},
			{ AST_LIKE,	10},
			{ AST_LSHT,	8},
			{ AST_RSHT,	8},
			{ AST_BIT_XOR,	7},
			{ AST_LOG_OR,	3},
			{ AST_LOG_AND,	4},
			{ AST_LOG_XOR,	5},
		},
	}, {
		AST_LSHT, {
			{ AST_NUMBER,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	12},
			{ AST_SUB,	12},
			{ AST_LIKE,	12},
		},
	}, {
		AST_RSHT, {
			{ AST_NUMBER,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	12},
			{ AST_SUB,	12},
			{ AST_LIKE,	12},
		},
	}, {
		AST_BIT_XOR, {
			{ AST_NUMBER,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	12},
			{ AST_SUB,	12},
			{ AST_LIKE,	12},
		},
	}, {
		AST_LOG_OR, {
			{ AST_NUMBER,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	12},
			{ AST_SUB,	12},
			{ AST_LIKE,	12},
			{ AST_LOG_NOT,	7},
		},
	}, {
		AST_LOG_AND, {
			{ AST_NUMBER,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	12},
			{ AST_SUB,	12},
			{ AST_LIKE,	12},
			{ AST_LOG_NOT,	7},
		},
	}, {
		AST_LOG_XOR, {
			{ AST_NUMBER,	11},
			{ AST_IDENTIFIER,	11},
			{ AST_ADD,	12},
			{ AST_SUB,	12},
			{ AST_LIKE,	12},
			{ AST_LOG_NOT,	7},
		},
	}, {
		AST_LOG_NOT, {
			{ AST_UNKNOWN,	7},
			{ AST_NEWLINE,	0},
			{ AST_ASSIGN,	7},
			{ AST_LOG_OR,	3},
			{ AST_LOG_AND,	4},
			{ AST_LOG_XOR,	5},
		},
	}, 
};
#endif /* __ZERG_PARSING_TABLE_H__ */
