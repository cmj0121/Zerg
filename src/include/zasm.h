/* Copyright (C) 2014-2016 cmj. All right reserved. */
#ifndef __ZASM_H__
#  define __ZASM_H__

#include "utils.h"

#define ZASM_VERSION		"1.0"
#define ZASM_ENTRY_POINT	".zasm.entry." ZASM_VERSION
#define TOKEN_ENTRY         "ENTRY"
#define TOKEN_ASM           "asm"
#define ZASM_INCLUDE		"include"
#define ZASM_MEM_BYTE		"byte"
#define ZASM_MEM_WORD		"word"
#define ZASM_MEM_DWORD		"dword"
#define ZASM_MEM_QWORD		"qword"

#include "zasm/instruction.h"
#include "zasm/binary.h"

typedef struct _tag_zasm_args_ {
	bool		pie;
	bool		symbol;
	std::string	dst;
} ZasmArgs;

class Zasm : public Binary {
	public:
		Zasm(ZasmArgs args) : Binary(args.dst, args.pie), _linono_(1), _args_(args) {};

		void assemble(std::string srcfile, off_t entry=0x100000);
	private:
		int _linono_;
		ZasmArgs _args_;
};

#endif /* __ZASM_H__ */

