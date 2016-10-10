# Lexer in Zerg #

詞法分析 (Lexer Analysis) 在 Zerg 中是一個簡單部分：Zerg 只提供少數合法的字符。
在 Zerg 的語法中，除了特定的運算符與保留字外，只提供三種額外的字符定義：數字、字串、字符。

Lexer analysis is a simple part in Zerg: it provides few validate tokes. The grammar of the Zerg
only provide three extra tokens: NUMBER, STRING and IDENTIFIER, except special operators and
reserved words defined in the grammar.

數字表示為一個合法的整數或者浮點數，它可以是用 2 進位、8 進位、10 進位、16 進位或科學方式來表示。
而字串則是用成對的單引號 (single quote) 或雙引號 (double quote) 的字符。剩下的情況則通稱為字符：
字符本身開頭為一個非數字、運算符可視字元集合。


The NUMBER is used to represent a valid integer or float number, it can be binary, octal, decimal,
hexadecimal or scientific notation. And the STRING is a token in the in-pairs of single-quote or
double-quote. Other case is call IDENTIFIER, which is starts with non-digit, non-operator printable
word sets.
