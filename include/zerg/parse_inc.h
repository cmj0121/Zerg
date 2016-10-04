#ifndef __ZERG_PARSING_TABLE_H__
	#define __ZERG_PARSING_TABLE_H__
/* GRAMMAR rules
 * 
 *   assign -> IDENTIFIER '=' expr 
 *     atom -> INT 
 *     atom -> IDENTIFIER 
 *     atom -> '+' atom 
 *     atom -> '-' atom 
 *     expr -> pow 
 *     expr -> pow '+' pow 
 *     expr -> pow '-' pow 
 *      pow -> atom 
 *      pow -> atom '*' atom 
 *      pow -> atom '/' atom 
 *      pow -> atom '%' atom 
 *     stmt -> NEWLINE 
 *     stmt -> assign NEWLINE 
 *     stmt -> expr NEWLINE 
 */

/* Parsing Table
 *
 *             NEWLINE     INT         IDENTIFIER  '='         '+'         '-'         '*'         '/'         '%'         
 * stmt        0           4           4                       5           5                                               
 * INT         0                                               2           2           3           3           3           
 * IDENTIFIER  0                                   1           2           2           3           3           3           
 * '='                     4           4                       5           5                                               
 * '+'         0           4           4           5           2           2           3           3           3           
 * '-'         0           4           4           5           2           2           3           3           3           
 * '*'                     4           4                       5           5                                               
 * '/'                     4           4                       5           5                                               
 * '%'                     4           4                       5           5                                               
 */

std::map<ASTType, std::map<ASTType, int>> _table_ = {
	{
		AST_UNKNOWN, {
			{ AST_NEWLINE,	0},
			{ AST_NUMBER,	4},
			{ AST_IDENTIFIER,	4},
			{ AST_ADD,	5},
			{ AST_SUB,	5},
		},
	}, {
		AST_NUMBER, {
			{ AST_NEWLINE,	0},
			{ AST_ADD,	2},
			{ AST_SUB,	2},
			{ AST_MUL,	3},
			{ AST_DIV,	3},
			{ AST_MOD,	3},
		},
	}, {
		AST_IDENTIFIER, {
			{ AST_NEWLINE,	0},
			{ AST_ASSIGN,	1},
			{ AST_ADD,	2},
			{ AST_SUB,	2},
			{ AST_MUL,	3},
			{ AST_DIV,	3},
			{ AST_MOD,	3},
		},
	}, {
		AST_ASSIGN, {
			{ AST_NUMBER,	4},
			{ AST_IDENTIFIER,	4},
			{ AST_ADD,	5},
			{ AST_SUB,	5},
		},
	}, {
		AST_ADD, {
			{ AST_UNKNOWN,	5},
			{ AST_NEWLINE,	0},
			{ AST_NUMBER,	4},
			{ AST_IDENTIFIER,	4},
			{ AST_ASSIGN,	5},
			{ AST_ADD,	2},
			{ AST_SUB,	2},
			{ AST_MUL,	3},
			{ AST_DIV,	3},
			{ AST_MOD,	3},
		},
	}, {
		AST_SUB, {
			{ AST_UNKNOWN,	5},
			{ AST_NEWLINE,	0},
			{ AST_NUMBER,	4},
			{ AST_IDENTIFIER,	4},
			{ AST_ASSIGN,	5},
			{ AST_ADD,	2},
			{ AST_SUB,	2},
			{ AST_MUL,	3},
			{ AST_DIV,	3},
			{ AST_MOD,	3},
		},
	}, {
		AST_MUL, {
			{ AST_NUMBER,	4},
			{ AST_IDENTIFIER,	4},
			{ AST_ADD,	5},
			{ AST_SUB,	5},
		},
	}, {
		AST_DIV, {
			{ AST_NUMBER,	4},
			{ AST_IDENTIFIER,	4},
			{ AST_ADD,	5},
			{ AST_SUB,	5},
		},
	}, {
		AST_MOD, {
			{ AST_NUMBER,	4},
			{ AST_IDENTIFIER,	4},
			{ AST_ADD,	5},
			{ AST_SUB,	5},
		},
	}, 
};
#endif /* __ZERG_PARSING_TABLE_H__ */
