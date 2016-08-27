# Zerg #

Zerg 是一個可編譯的程式語言：用來讓我了解程式語言的概念，以及如果產生可執行檔的方法。
在 Zerg (v1) 中他是設計執行在 Mac OS X 的環境，這表示他現階段只能夠在 Mac OS X 上運作。
而底層是基於 zasm，他是一個組譯器，設計用來給 zerg 來產生可執行使用。

Zerg is a compiled programming language which is used to let me know the concept of the programming
language and how the binary work via a compiler.
Zerg (v1) is designed on the Mac OS X, this means zerg only workable on Mac environment and
based on zasm, it is a assembler designed for zerg, to produce a binary running on Mac.

## 設計流程 / Design Flow ##
如同 commit 06eb66e 上的說法，Zerg 已經被我重新設計過很多次了，在這些設計過程中犯了不少錯誤，
也修改的不少實作的方式。在這個版本中，依然使用 C++ (c++11) 來實作第一版的 Zerg，
並且最終希望可以用這一版 Zerg 編譯出其他版的 Zerg 而達到 self-hosting 的目的。

這次實作的方式利用 Top-Down 的流程來完成 Zerg：也就是先假設底層功能已經完成 (像是
Parser 與 AST 等)，先完成一個最小有價值產品 (Minimal Value Product, MVP)，並且評估是否有過度設計
(Over-engineering) 的狀況發生。

根據編譯器 (compiler) 在產生最終可執行檔時，會經過若干個過程：詞彙分析 (lexical analysis)、
語法分析 (syntax analysis)、語意分析 (semantic analysis)、中介語言
(intermediate representation, IR)、代碼產生 (code generation)。而在 IR 階段，
可以幾過若干最佳化後產生不同的 IR，進而產生更加的機械語言。這裡的最佳可以是大小、速度或平台相容等。


### 射出器 / Emitter ###
從 IR 到 code generation 會經由射出器 (emitter) 來橋接兩個階段。
在這個階段會經由一個 emitter，將 IR 射出成相對的代碼 (機械碼)。對於 IR 來說是平台無關
(platform-independent) 的一個語言，代表的是低階的程式操作，像是暫存器 (register) 的運算、
寫入到記憶體 (memory) 或者是系統中斷 (interrupt) 等。然而，對於不同平台會有不同的支援程度，
像是 x86-64 擁有 16 個通用暫存器，但是 ARM A8 卻擁有 31 個通用暫存器。

因此 IR 的設計會假設擁有平台無關的特性：無限多個暫存器、支援所有運算與浮點運算等。
根據 IR 的設計，可能會偏向原本程式語言的型態 (High-Level IR) 或者是偏向目標語言
(Low-Level IR)，每種 IR 都有相對應的目的與最佳化的方式。而我所設計的 Zasm 也可以當作是一種 IR：
可以藉由 Zasm 的語法來產生最終的機械碼與可執行檔。然而 Zasm 本身指定使用特定的暫存器，這跟 IR
的設計目的有相違背，因此在 Zerg 會先設計出 Zasm+：這是一個基於 Zasm 語法的 IR 但擁有無限數量的
register。在 Zasm+ 中用 r 開頭表示 64-bits 大小的 register 並且接續數字表示使用第 n 個暫存器：

+ r12 表示使用第 12 個通用暫存器，size 為 64-bits
+ w12 為 32-bits
+ d12 為 16-bits
+ b12 為 8-bits

而 Zasm+ 使用 4-tuple 來呈現：也就是使用 (operator, dst, src, extra) 來表示一個虛擬指令，
這個指令也就是 dst = dst operator src with extra 的用法。使用 4-tuple 的原因如下：

+ 所有的虛擬指令都代表著一個行為，也就是 operation
+ 虛擬指令至多只有一個結果需要儲存，也就是 dst
+ 虛擬指令最多允許兩個運算元，以減少問題的複雜程度。

### Low-Level IR ###
在這個 IR 設計中，operator 盡可能貼近到底層的操作而不做過多的包裝。因此只使用下述 40 種運算子，
這些運算子本身提供最低限度的邏輯來呈現一個程式，包含值之間傳遞、基本的數值運算、位元運算、
條件判斷、函數呼叫、整數與浮點數的轉換、標籤與系統中斷。


	:::
	COPY	LOAD	STORE
	ADD		SUB		MUL		DIV		REM		INC		DEC
	SHL		SHR		AND		OR		XOR		NOT		NEG		ABS
	CMP		EQ		NEQ		LS		LSE		GT		GTE
	JMP		JEQ		JNEQ	JLS		JLSE	JGT		JGTE
	CALL	RET		PARAM
	ITOF	FTOI
	LABEL	NOP		INTERRUPT

然而對應到底層的 emitter 中，同一個指令配上不同的運算元則會得到不一樣的結果。像是
COPY R1 0x12 與 COPY R1 R2 對於底層機械碼來說，是使用不同的 opcode。而這部分的邏輯就交由
emitter 處理產生正確的 opcode。
在這個設計中，運算元可以是任意數字、浮點數、字串、暫存器、記憶體位址與 STACK 等。

在這個階段的 IR，主要需要處理的是從無限數量的暫存器轉換成特定數量的暫存器來使用。
這個問題等價於圖論中的著色問題：任意相連的兩個區塊顏色不能一樣。在 IR 領域中就表示同一個虛擬指令中，
不同暫存器轉換後使用到的暫存器不能一樣。

