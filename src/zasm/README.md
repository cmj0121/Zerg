Zasm 是由我設計的用來給 zerg 使用的直譯器 (assembler)，除了支援 (部分) 基本的組合語言外，
還額外支援引用 (reference) 概念的符號。

Zasm 支援一般用途的字串 (用單引號或雙引號) 以及數字 (十進位獲十六進位)、平台相關的指令集與暫存器，
和特徵符號。特徵符號是一個特別的概念，跟標籤很像但是只能用來當作是引用。他可以被用來當作是函數的進入點，
或者是變數的參照 (目前只支援字串)。註解是使用井字號 (#) 當作開頭並且結尾為換行符號 ('\n')。

你可以從 [zasm.vim](https://github.com/cmj0121/Zerg/blob/master/zerg.vim) 這個檔案中獲得更多的資訊。


Zasm is a assembler which I designed to be used for zerg. It supports (partial) basic assembly
language and the concept of the reference.

Zasm supports string (using single quote or double quote) and digit (decimal or hexadecimal)
for the general purpose, the platform-related instruction and registers and identifier.
The identifier is the spacial and used like a label, but it is used as the reference which can
be used as the entry point of the function, or the symbol of the variable (now only support string).
The comment is starts with the hash-mark (#) and ends with the end-of-line ('\n').

You can see more detail in the [zasm.vim](https://github.com/cmj0121/Zerg/blob/master/zerg.vim).


## 語法 / GRAMMAR ##

在這個版本中的 zasm，支援的平台只有 Max OS X / x86-64. 這表示 zasm 現在只可以在這個特定平台使用。
在 zasm 中是根據每一行的內容來處理指令集。這表示你不能用多行來表示一個指令集。
而每一個被處理的 token 都是用貢白字源分隔。

不驚訝的，zasm 目前只有支援常見的指令集 (像是 mov 以及 syscall)，支援基本的暫存器 (像是 rax
以及 r15)。而額外的保留字 'asm' 則是用來定義一個代表符號：根據使用方式，他可以是函數的進入點或是一個變數。

+ 當開頭為 'asm' 以及結尾是冒號 (':')，代表這個符號是用來表示函數的進入點，或者你可以視為是一個標籤。注意，
使用上他只能用兩個 token 來表示。
+ 當開頭為 'asm' 以及擁有三個 token 則表示用來定義一個符號以及擁有的特殊值 (目前只支援字串)。



In this version, zasm support Max OS X / x86-64 only. This means zasm is only workable on one
of the particular platform. In the zasm, assemble the instruction line-by-line. It means you
cannot write one instruction more than one line. And the token is separated by the whitespace.

No surprised, zasm support the common instructions, like mov and syscall, and support basic
registers, like rax and r15. And more, the reserved word 'asm' is used to define a identifier:
it may be a function entry point or the variable, dependent on the usage.

+ Starts with 'asm' and ends with colon (':') means this identifier is used as the function entry
point, or you can simplify treat as the label. NOTE, it only supports with two tokens.
+ Starts with 'asm' and has three tokens is means define the identifier with special value (now
only support string)


## 架構 / Architecture ##

Zasm 是一個基於 C++ 的直譯器，進入點可以從檔案
[src/zasm.cpp](https://github.com/cmj0121/Zerg/blob/master/src/zasm.cpp) 中的 main 函數開始。
經過若干的參數檢查以及儲利後，zasm 只需要呼叫 Zasm 的建構子以及他的函數 (compile) 就可以產生可執行檔。

Zasm 本身擁有兩個部分：一個是從組合語言產生的機械馬的直譯器，另一個則是產生合法有效的可執行檔格式。
在目前的版本中 (v1)，目前只支援
[Mach-O 64](https://developer.apple.com/library/mac/documentation/DeveloperTools/Conceptual/MachORuntime/index.html)
格式。在這個檔案中，主要的部分落在
[L#63-L#70](https://github.com/cmj0121/Zerg/blob/master/src/zasm/macho64.cpp#L63) 上，
而這個部分會呼叫兩次來產生必須的 Mach-O 格式的標頭。

除了產生標頭之外，機械碼的產生也是重要且很難時做的部分。在標頭檔中
The header file [include/zasm.h](https://github.com/cmj0121/Zerg/blob/master/include/zasm.h)
提供了若干個 class：

+ Zasm 是一個前端的介面處理器
+ Binary 用來產生一個合法格式的可執行檔
+ Instruction 根據每一行的指令產生機械碼
+ InstToken 處理指令符並且提供相對的狀態讓 Instruction 使用

在 class 'Zasm' 中，他擁有兩個內部函數：一個用來把組合語言分為若干個合法的指令符，
另一個則是將每一行的指令符送給下一個 class (Instruction) 並且將機械碼純在一個陣列中。
class 'Binary' 則是依平台來產生合法的可執行檔。他簡單但瑣碎的根據平台來產生合法格式的檔案。

而關於 class 'Instruction' 則勢將組合語言直譯為機械碼的一個部分，內部有一個抽象函數 'assemble'：
從平台相關的指令集中依序尋找出符合的指令，並產生機械碼。已 x86-64 平台為例，一個指令集最多有
15 bytes，而且可以分為五個部分來處理。


Zasm is C++-based assembler which starts on the main function in the file
[src/zasm.cpp](https://github.com/cmj0121/Zerg/blob/master/src/zasm.cpp).
After several parameter check and process, zasm only call the instruction of the class Zasm
and run one member function, called compile, and generate a executable binary.

Zasm has two part to generate the binary: one is compile the assemble language into the machine
code, and another is process the machine code into valid binary format. In the current version (v1),
only support the
[Mach-O 64](https://developer.apple.com/library/mac/documentation/DeveloperTools/Conceptual/MachORuntime/index.html)
format, and implemented on the [src/zasm/macho64.cpp](https://github.com/cmj0121/Zerg/blob/master/src/zasm/macho64.cpp).
In this file, the main-part is located on the
[L#63-L#70](https://github.com/cmj0121/Zerg/blob/master/src/zasm/macho64.cpp#L63) which call
twice to generate the necessary Mach-O header.

Besides generated the header, machine code generation is the important and implemented hardly.
The header file [include/zasm.h](https://github.com/cmj0121/Zerg/blob/master/include/zasm.h)
give several class:

+ Zasm is the front-end interface.
+ Binary is used to generate the valid format binary
+ Instruction assembly machine code line-by-line
+ InstToken process the assembly token and reply the status when need

In class 'Zasm', it has two member function, one is token which separate the assembly into
valid token, another is compile which put the tokens into next class (Instruction) and
store into a array.The class 'Binary' is used to generate valid format binary on each platform.
It is trivial and just dump the particular format for each platform.

Then the part of the class 'Instruction' is used to assemble from the instruction into machine code,
which has a virtual function 'assemble' and iterate the list of the instruction set, find out the
suitable instruction and assemble to machine code related on each platform. For example, the x86-64
, it has max to 15 bytes and can be separated into five difference parts.


## example ##

	:::zasm
	asm __main__:
		call &banner		# jump to function 	0x05

		mov rax 0x2000001	# exit				0x07
		mov rdi 0x04		# return value		0x07
		syscall				# run syscall		0x02
	asm banner:
		mov rax 0x2000004	# sys_write			0x07
		mov rdi 0x02		# stderr			0x07
		lea rsi &Banner		# string			0x07
		mov rdx 0x35		# size of string	0x07
		syscall				# run syscall		0x02

		mov rax 0x2000004	# sys_write			0x07
		mov rdi 0x01		# stdout			0x07
		lea rsi	&str		# string for Banner	0x07
		mov rdx 0x0C		# size of string	0x07
		syscall				# run syscall		0x02
		ret					# return			0x01

	asm str "Hello World\n"
	asm Banner "Welcome to ZASM - A prototype for a simple assembler\n"
