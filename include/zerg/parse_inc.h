/* GRAMMAR rules
 * 
 *       and_test -> xor_test 
 *       and_test -> xor_test 'and' xor_test 
 *
 *           args -> test 
 *           args -> test ',' test 
 *
 *          arith -> pow 
 *          arith -> pow '+' pow 
 *          arith -> pow '-' pow 
 *
 *         assign -> IDENTIFIER '=' expr 
 *
 *           atom -> NUMBER 
 *           atom -> STRING 
 *           atom -> IDENTIFIER 
 *           atom -> '+' NUMBER 
 *           atom -> '+' STRING 
 *           atom -> '+' IDENTIFIER 
 *           atom -> '-' NUMBER 
 *           atom -> '-' STRING 
 *           atom -> '-' IDENTIFIER 
 *           atom -> '~' NUMBER 
 *           atom -> '~' STRING 
 *           atom -> '~' IDENTIFIER 
 *
 *        bit_and -> bit_xor 
 *        bit_and -> bit_xor '&' bit_xor 
 *
 *         bit_or -> bit_and 
 *         bit_or -> bit_and '|' bit_and 
 *
 *        bit_xor -> shift 
 *        bit_xor -> shift '^' shift 
 *
 *           expr -> funccall 
 *           expr -> or_test 
 *
 *       funccall -> IDENTIFIER '(' ')' 
 *       funccall -> IDENTIFIER '(' args ')' 
 *       funccall -> 'syscall' '(' ')' 
 *       funccall -> 'syscall' '(' args ')' 
 *
 *       not_test -> bit_or 
 *       not_test -> 'not' bit_or 
 *
 *        or_test -> and_test 
 *        or_test -> and_test 'or' and_test 
 *
 *            pow -> atom 
 *            pow -> atom '*' atom 
 *            pow -> atom '/' atom 
 *            pow -> atom '%' atom 
 *            pow -> atom '~' atom 
 *
 *          shift -> arith 
 *          shift -> arith '<<' arith 
 *          shift -> arith '>>' arith 
 *
 *           stmt -> NEWLINE 
 *           stmt -> assign NEWLINE 
 *           stmt -> expr NEWLINE 
 *
 *           test -> or_test 
 *
 *       xor_test -> not_test 
 *       xor_test -> not_test 'xor' not_test 
 *
 */

std::map<ASTType, std::map<ASTType, int>> _table_ = {
	{
		AST_ROOT, {
			{ AST_NEWLINE,	0},
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
			{ AST_LOG_NOT,	9},
			{ AST_SYSCALL,	3},
		},
	}, {
		AST_NUMBER, {
			{ AST_NEWLINE,	0},
			{ AST_PARENTHESES_CLOSE,	3},
			{ AST_COMMA,	4},
			{ AST_ADD,	14},
			{ AST_SUB,	14},
			{ AST_MUL,	15},
			{ AST_DIV,	15},
			{ AST_MOD,	15},
			{ AST_LIKE,	15},
			{ AST_LSHT,	13},
			{ AST_RSHT,	13},
			{ AST_BIT_OR,	10},
			{ AST_BIT_AND,	11},
			{ AST_BIT_XOR,	12},
			{ AST_LOG_OR,	6},
			{ AST_LOG_AND,	7},
			{ AST_LOG_XOR,	8},
		},
	}, {
		AST_STRING, {
			{ AST_NEWLINE,	0},
			{ AST_PARENTHESES_CLOSE,	3},
			{ AST_COMMA,	4},
			{ AST_ADD,	14},
			{ AST_SUB,	14},
			{ AST_MUL,	15},
			{ AST_DIV,	15},
			{ AST_MOD,	15},
			{ AST_LIKE,	15},
			{ AST_LSHT,	13},
			{ AST_RSHT,	13},
			{ AST_BIT_OR,	10},
			{ AST_BIT_AND,	11},
			{ AST_BIT_XOR,	12},
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
			{ AST_ADD,	14},
			{ AST_SUB,	14},
			{ AST_MUL,	15},
			{ AST_DIV,	15},
			{ AST_MOD,	15},
			{ AST_LIKE,	15},
			{ AST_LSHT,	13},
			{ AST_RSHT,	13},
			{ AST_BIT_OR,	10},
			{ AST_BIT_AND,	11},
			{ AST_BIT_XOR,	12},
			{ AST_LOG_OR,	6},
			{ AST_LOG_AND,	7},
			{ AST_LOG_XOR,	8},
		},
	}, {
		AST_PARENTHESES_OPEN, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_PARENTHESES_CLOSE,	3},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
			{ AST_LOG_NOT,	9},
		},
	}, {
		AST_PARENTHESES_CLOSE, {
			{ AST_NEWLINE,	0},
		},
	}, {
		AST_COMMA, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
			{ AST_LOG_NOT,	9},
		},
	}, {
		AST_ASSIGN, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
			{ AST_LOG_NOT,	9},
			{ AST_SYSCALL,	3},
		},
	}, {
		AST_ADD, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
		},
	}, {
		AST_SUB, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
		},
	}, {
		AST_MUL, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
		},
	}, {
		AST_DIV, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
		},
	}, {
		AST_MOD, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
		},
	}, {
		AST_LIKE, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
		},
	}, {
		AST_LSHT, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
		},
	}, {
		AST_RSHT, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
		},
	}, {
		AST_BIT_OR, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
		},
	}, {
		AST_BIT_AND, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
		},
	}, {
		AST_BIT_XOR, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
		},
	}, {
		AST_LOG_OR, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
			{ AST_LOG_NOT,	9},
		},
	}, {
		AST_LOG_AND, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
			{ AST_LOG_NOT,	9},
		},
	}, {
		AST_LOG_XOR, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
			{ AST_LOG_NOT,	9},
		},
	}, {
		AST_LOG_NOT, {
			{ AST_NUMBER,	16},
			{ AST_STRING,	16},
			{ AST_IDENTIFIER,	16},
			{ AST_ADD,	16},
			{ AST_SUB,	16},
			{ AST_LIKE,	16},
		},
	}, {
		AST_SYSCALL, {
			{ AST_PARENTHESES_OPEN,	3},
		},
	}, 
};

