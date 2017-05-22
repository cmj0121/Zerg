/* Copyright (C) 2014-2016 cmj. All right reserved. */
#ifndef __ZASM_H__
#  define __ZASM_H__

#include "utils.h"

#define ZASM_VERSION		"1.1"
#define ZASM_ENTRY_POINT	".zasm.entry"

#define ZASM_DEFINE			"define"
#define ZASM_INCLUDE		"include"

#define ZASM_MEM_BYTE		"byte"
#define ZASM_MEM_WORD		"word"
#define ZASM_MEM_DWORD		"dword"
#define ZASM_MEM_QWORD		"qword"


#include <map>
#include "zasm/instruction.h"
#include "zasm/binary.h"

typedef struct _tag_zasm_args_ {
	bool		pie;
	bool		symbol;
	off_t		entry;
	std::string	dst;
} ZasmArgs;

class Zasm : public Binary {
	public:
		Zasm(ZasmArgs args) : Binary(args.dst, args.pie), _args_(args) {};
		virtual ~Zasm(void) {
			/* Output the binary */
			Binary::dump(this->_args_.entry, this->_args_.symbol);
		}
	private:
		ZasmArgs _args_;
};

#endif /* __ZASM_H__ */

