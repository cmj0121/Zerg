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


#define VALID_SYMBOL(_symb_)	\
	(_symb_ == ZASM_ENTRY_POINT || \
	 ("" != _symb_ && '.' != _symb_[0] && '_' != _symb_[0]))


class ZasmToken {
	public:
		ZasmToken() : _src_("") {};
		ZasmToken(std::string src);

		bool isREG(void);
		bool isPosREG(void);
		bool isMEM(void);
		bool isMEM2(void);	/* Memory with two registers */
		bool isIMM(void);
		bool isEXT(void);
		bool isREF(void);
		bool isSSE(void);	/* Streaming SIMD Extensions */

		ZasmToken* asReg(void);
		ZasmToken* indexReg(void);
		off_t asInt(void);
		off_t offset(void);

		int size(void);

		std::string raw(void);
		std::string unescape(void);

		bool match(unsigned int flag);
		bool operator== (std::string src);
		bool operator!= (std::string src);
		operator int() const;
	private:
		std::string _src_;
};
static ZasmToken EMPTY_TOKEN("");

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

		void compile(std::string srcfile);
		ZasmToken* token(std::fstream &src);
	private:
		int _linono_;
		ZasmArgs _args_;
};

#endif /* __ZASM_H__ */

