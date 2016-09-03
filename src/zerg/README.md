# Zerg #

Zerg 是一個可編譯的程式語言：用來讓我了解程式語言的概念，以及如果產生可執行檔的方法。
在 Zerg (v1) 中他是設計執行在 Mac OS X 的環境，這表示他現階段只能夠在 Mac OS X 上運作。
而底層是基於 zasm，他是一個組譯器，設計用來給 zerg 來產生可執行使用。

Zerg is a compiled programming language which is used to let me know the concept of the programming
language and how the binary work via a compiler.
Zerg (v1) is designed on the Mac OS X, this means zerg only workable on Mac environment and
based on zasm, it is a assembler designed for zerg, to produce a binary running on Mac.

就實作層面來說，前端會有一個 Zerg 的 class 用來處理詞彙、語法與語句分析，並且轉換成 IR 格式。
IR 格式則是一個中介語言，用來處理各種最佳化與平台相關的處理。最後則用 IR 的 emit 函數，
藉由 Zasm 來產生最終平台相關的機械碼。

When implement, Zerg has a front-end interface, call Zerg, used to process the lexical
analysis, syntax analysis and semantic analysis, and then translate to IR. IR is a middle-layer
in Zerg which is used to process any optimizations and platform operations. After this stage,
Zasm will generate the finial platform-dependent machine codes by call the function 'emit' in IR.


## 設計流程 / Design Flow ##
如同 commit 06eb66e 上的說法，Zerg 已經被我重新設計過很多次了，在這些設計過程中犯了不少錯誤，
也修改的不少實作的方式。在這個版本中，依然使用 C++ (c++11) 來實作第一版的 Zerg，
並且最終希望可以用這一版 Zerg 編譯出其他版的 Zerg 而達到 self-hosting 的目的。

Same in the commit 06eb66e, Zerg is been redesigned many times by me and I make several mistake
when design and implement. In this version of Zerg, I still using C++ (c++11) to implement the
first version of Zerg, and hope it is the last version Zerg which can compile another Zerg to
fit the purpose of self-hosting.

這次實作的方式利用 Top-Down 的流程來完成 Zerg：也就是先假設底層功能已經完成 (像是
Parser 與 AST 等)，先完成一個最小有價值產品 (Minimal Value Product, MVP)，並且評估是否有過度設計
(Over-engineering) 的狀況發生。

In time using Top-Down to implement Zerg: assume the low-end functionality is full-complete,
like, parser and AST, to build a MVP (minimal value produce) and evaluate this implement has
over-engineering or not.


根據編譯器 (compiler) 在產生最終可執行檔時，會經過若干過程：詞彙分析 (lexical analysis)、
語法分析 (syntax analysis)、語意分析 (semantic analysis)、中介語言
(intermediate representation, IR)、代碼產生 (code generation)。在代碼產生階段，我們透過
Zasm 來產生最終的程式碼。

There are several stages when compiling: lexical analysis, syntax analysis, semantic analysis,
IR, code generation. In the stage of code generation, using Zasm to produce the final binary.

## 中介語言 / IR ##
IR 是一種中介語言同時也會是一種程式語言：本身提供較少的語法與簡潔的變數運用。
因此每個程式語言都可以是更高階語言的 IR。像是 C 語言本身可以是更高階的程式語言，
藉由將程式碼轉成 C 語言之後編譯成最終的可執行檔。根據 IR 的性質，可能偏向最終的機械碼
(low-level IR) 或者是偏向原本的程式碼 (high-level IR)。

IR is an intermediate language and is a programming language too: provide few grammar and
more clear variable declare and usage, so each programming language also is an IR for the
higher level programming language. For example, C can be an IR for the high-level
programming language. Based on the properties of IR, it can be more like finial machine
code (low-level IR) or more like original source code (high-level IR).

需要 IR 的原因在可以簡化原本程式碼的邏輯。在原本的程式碼可能包含了機械碼所無法簡單做到的功能，
像是無限數量的變數、平行運算、複雜的條件判斷式與數學運算等。IR 則將這些困難操作轉化成若干較簡單的操作
，並針對平台無關的指令做最佳化。但是在這個版本中的 Zerg 並不處理任何最佳化的處理。

The reason for using IR is to simply the program logical. In the original, programming language
may contains several functionality which machine code cannot be implemented easily, like
lots of variable, complexity condition and arithmetic. IR can transfer from complexity operations
into several simpler and manageable one, and more can perform machine independent optimization.
But in this version, Zerg does NOT process any optimization.

### High-Level IR ###
經過詞彙、語法分析完之後 Zerg 就會產生一個抽象語法樹 (Abstract Syntax Tree, AST) 與
流程圖 (Control Flow Graph, CFG)。根據轉換後的 AST 與 CFG，Zerg 可以將原本的程式碼轉譯成新的程式語言
IR。在這個階段的 IR，主要的目的是將原本程式的複雜邏輯初步的轉換成若干的簡單邏輯，像是：

+ 無限制的變數使用與記憶體空間變成受限制的。
+ 簡單的賦值表示式。
+ 流程控制與條件式簡單化。

After the lexical and syntax analysis, Zerg will generate an AST (Abstract Syntax Tree) and CFG
(Control Flow Graph). By the AST and CFG, Zerg will compile the original source code into
new programming language: IR. In this stage, the purpose of IR is translate the complexity logical
into several simpler one, like

+ Unlimited variables and memory usage into limit one.
+ Simpler variable assignment.
+ Simpler flow control and condition expression.


### Low-Level IR ###
在這個 IR 設計中，提供的運算子盡可能貼近到底層的操作而不做過多的包裝。因此只使用下述 41 種運算子，
這些運算子本身提供最低限度的邏輯來呈現一個程式，包含值之間傳遞、基本的數值運算、位元運算、
條件判斷、函數呼叫、整數與浮點數的轉換、標籤與系統中斷。

The operators used in low-level IR is more like the assemble language but without any
platform-dependent instructions. So the following 41 operators is the used in this stage. These
operators provide the lower logical to describe a simple logical, like variable copy, basic
arithmetic and bitwise operation, condition check, function call, transfer between integer and float,
label and system interrupt.

	:::
	COPY	LOAD	STORE
	ADD		SUB		MUL		DIV		REM		INC		DEC
	SHL		SHR		AND		OR		XOR		NOT		NEG		ABS
	CMP		EQ		NEQ		LS		LSE		GT		GTE
	JMP		JEQ		JNEQ	JLS		JLSE	JGT		JGTE
	CALL	RET		PARAM
	ITOF	FTOI
	LABEL	NOP		INTERRUPT
	ASM


