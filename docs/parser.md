# Parser in Zerg #

語法分析在編譯器領域中一個困難的部分：根據語法定義的不同、分析器的實作方式可能會遇到不一樣的問題。
Zerg 的語法用 BNF (Backus–Naur Form) 相似的定義方式，轉換成較簡單的語法定義並產生一個分析表
(Parsing Table)。分析表本身為一個 LL(1) 的方式來解析：需要使用當下與前一個字符才能得到這個字符的權重，
而權重本身是用來產生抽象語法樹 (AST, Abstract Syntax Tree) 使用。

Parsing or syntactic analysis in compiler is a difficult-part: related on the grammar definition,
the implement of the parser, it may have difference problem. Zerg is used the BNF (Backus-Naur Form)
like format to define the grammar, transfer into several simpler grammar definition and generate
a parsing table. The parsing table is a LL(1) parsing table: need the current token and previous token
to get the weight of the token, and the weight is used to generate the AST (Abstract Syntax Tree).

舉例來說：當需要分析 `1 + 2 * 3` 是否是一個合法語法時，會使用到下列的分析表並且得到權重。
當無法找到相對的權重時，則表示這是個不合法的語法：

For example, when analysis `1 + 2 * 3` is valid or not, using the following parsing table and get the
weight, and it is a invalid grammar if cannot find out the weight in the table.

	:::
	stmt  -> [ stmt ( '+' | '-' ) ] pow
	pow   -> [ pow  ( '*' | '/' ) ] NUMBER

	/*
	 * stmt           -> pow
	 * stmt           -> stmt '+' pow
	 * stmt           -> stmt '-' pow
	 *
	 * pow            -> NUMBER
	 * pow            -> pow '*' NUMBER
	 * pow            -> pow '/' NUMBER
	 */

	          NUMBER  '+'  '-'  '*'  '/'
	        +---------------------------
	ε       |  3
	NUMBER  |          2    2    1    1
	'+'     |  3
	'-'     |  3
	'*'     |  3
	'/'     |  3

