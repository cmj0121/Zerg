/* Copyright (C) 2014-2016 cmj. All right reserved. */
#ifndef __ZASM_H__
#  define __ZASM_H__

#include "utils.h"

#define ZASM_VERSION		"1.1"
#define ZASM_ENTRY_POINT	".zasm.entry"

#define ZASM_DEFINE			"define"
#define ZASM_INCLUDE		"include"

#define ZASM_REFERENCE		'&'
#define ZASM_CURRENT_POS	"$"
#define ZASM_SESSION_POS	"$$"

#define ZASM_MEM_BYTE		"byte"
#define ZASM_MEM_WORD		"word"
#define ZASM_MEM_DWORD		"dword"
#define ZASM_MEM_QWORD		"qword"


#include <map>
#include "zasm/instruction.h"

class Zasm {
	public:
		Zasm(std::string dst, Args &args) : _args_(args), _linono_(0), _dst_(dst) {};
		virtual ~Zasm(void);

		void dump(off_t entry = 0x1000, bool symb=false);	/* Binary-Specified */

		void assembleF(std::string srcfile);
		void assembleL(std::string line);

		Zasm& operator+= (Instruction *inst);
	private:
		Args _args_;
		int _linono_;
		std::string _dst_;
		std::vector<Instruction *> _inst_;
		std::map<std::string, std::string> _map_;

		void reallocreg(void);
		off_t length(void);
};

#endif /* __ZASM_H__ */

