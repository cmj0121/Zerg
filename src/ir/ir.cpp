/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include "zerg.h"

static std::map<std::string, IROP> IROP_map = {
	/* memory access*/
	{"STORE"	, IR_MEMORY_STORE},
	{"LOAD"		, IR_MEMORY_LOAD},
	{"XCHG"		, IR_MEMORY_XCHG},
	{"PUSH"		, IR_MEMORY_PUSH},
	{"POP"		, IR_MEMORY_POP},
	{"PARAM"	, IR_MEMORY_PARAM},
	/* arithmetic operation */
	{"ADD"		, IR_ARITHMETIC_ADD},
	{"SUB"		, IR_ARITHMETIC_SUB},
	{"MUL"		, IR_ARITHMETIC_MUL},
	{"DIV"		, IR_ARITHMETIC_DIV},
	{"MOD"		, IR_ARITHMETIC_MOD},
	{"SHR"		, IR_ARITHMETIC_SHR},
	{"SHL"		, IR_ARITHMETIC_SHL},
	{"INC"		, IR_ARITHMETIC_INC},
	{"DEC"		, IR_ARITHMETIC_DEC},
	/* logical operation */
	{"AND"		, IR_LOGICAL_AND},
	{"OR"		, IR_LOGICAL_OR},
	{"XOR"		, IR_LOGICAL_XOR},
	{"NOT"		, IR_LOGICAL_NOT},
	{"NEG"		, IR_LOGICAL_NEG},
	{"EQ"		, IR_LOGICAL_EQ},
	{"LS"		, IR_LOGICAL_LS},
	{"GT"		, IR_LOGICAL_GT},
	/* condition / control flow */
	{"JMP"		, IR_CONDITION_JMP},
	{"JMPIFN"	, IR_CONDITION_JMPIFN},
	{"CALL"		, IR_CONDITION_CALL},
	{"RET"		, IR_CONDITION_RET},
	/* extra */
	{"NOP"		, IR_NOP},
	{"PROLOGUE"	, IR_PROLOGUE},
	{"EPILOGUE"	, IR_EPILOGUE},
	{"INTERRUPT", IR_INTERRUPT},
	{"LABEL"	, IR_LABEL},
	{"DEFINE"	, IR_DEFINE},
	{"INLINE"	, IR_INLINE_ASM},
};

IR::IR(std::string dst, Args &args) : Zasm(dst, args) {
	this->_syscall_nr_	= 0;
	this->_lineno_		= 0;
	this->_args_		= args;
}

void IR::compile(std::string src) {
	std::string line;
	std::fstream fs(src);

	if (!fs.is_open()) {
		/* cannot open the source code */
		_D(LOG_CRIT, "source file `%s` does NOT exist", src.c_str());
	}

	while (std::getline(fs, line)) {
		/* compile IR line-by-line */
		IR::compileL(line);
	}

	/* compile the IR to Zasm */
	_D(LOG_DEBUG_LEXER, "Total #%zu IR", this->_irs_.size());
	if (this->_args_.only_ir || true) {
		#if defined(DEBUG_REGISTER_ALLOC)
			/* allocate registers */
			regalloc();
		#endif /* DEBUG_REGISTER_ALLOC */
		/* show the IR */
		for (auto ir : this->_irs_) {
			for (auto it : IROP_map) {
				if (it.second == ir.opcode) {
					if (it.second == IR_LABEL) std::cout << "\n";
					std::cout << std::setw(14) << std::left << it.first;
				}
			}

			if ("" != ir.dst)   std::cout << std::setw(24) << ir.dst;
			if ("" != ir.src)   std::cout << std::setw(24) << ir.src;
			if ("" != ir.size)  std::cout << std::setw(24) << ir.size;
			if ("" != ir.index) std::cout << std::setw(24) << ir.index;
			std::cout << std::endl;
		}
	} else {
		/* allocate registers */
		regalloc();
		/* generate the binary code via Zasm::dump */
		Zasm::dump();
	}
}

void IR::compileL(std::string line) {
	unsigned int pos, nr = 0;
	std::string irs[5] = {""}/* OPCODE DST SRC SIZE INDEX */, tmp;

	/* lexer IR line-by-line */
	for (unsigned cur = 0; cur <= line.size(); ++cur) {
		switch(line[cur]) {
			case '\0': case '#':
				/* Need not process anymore */
				goto EMIT;
			case ' ': case '\t':
				/* whitespace */
				break;
			case '"': case '\'':			/* String */
				tmp = line[cur++];
				while ('\0' != line[cur]) {
					tmp += line[cur];

					if (tmp[0] == line[cur]) break;
					++cur;
				}

				if (tmp[0] != line[cur]) {
					/* invalid syntax for string */
					_D(LOG_CRIT, "syntax error - %s", line.c_str());
				}
				irs[nr++] = tmp;
				break;
			default:
				for (pos = cur+1; pos <= line.size(); ++pos) {
					if (' ' == line[pos] || '\t' == line[pos]) break;
				}

				if (nr > ARRAY_SIZE(irs)) {
					_D(LOG_CRIT, "Invalid IR `%s`", line.c_str());
					break;
				}
				irs[nr++] = line.substr(cur, pos-cur);
				cur       = pos;
				break;
		}
	}
EMIT:
	if (0 != nr) {
		if (IROP_map.end() == IROP_map.find(irs[0])) {
			_D(LOG_CRIT, "Not implement IR - %s", irs[0].c_str());
		}

		_D(LOG_DEBUG_LEXER, "IR lexer L#%04d - %s", ++_lineno_, line.c_str());
		IRInstruction inst = {IROP_map[irs[0]], irs[1], irs[2], irs[3], irs[4], _lineno_};
		this->_irs_.push_back(inst);
	}
}

/* Register Allocation Problem
 *
 * In ZGR assume we have infinite number of registers, but in real-word we only have
 * limited number of register (e.g. 16 in X86-64 environment).
 *
 * we only can used the USED_REGISTERS defined in Zasm
 */
void IR::regalloc(void) {
	for (size_t i = 0; i < _irs_.size(); ++i) {
		_irs_[i].dst = this->regalloc(i, _irs_[i].dst);
		_irs_[i].src = this->regalloc(i, _irs_[i].src);
	}
}
std::string IR::regalloc(int irpos, std::string src) {
	bool blFound = false;
	int cnt = 0, pos = 0;
	std::string ret = src, size = _irs_[irpos].size;

	/* register provided from Zasm, platform-dependency */
	static std::vector<std::string> regs = { REGISTERS };

	size = "" == size ? ZASM_MEM_QWORD : size;
	if (1 == sscanf(src.c_str(), __IR_REG_FMT__, &cnt)) {	/* register */
		if (0 != _alloc_regs_map_.count(src)) {
			ret = _alloc_regs_map_[src];

			#if defined(DEBUG_REGISTER_ALLOC) || defined(DEBUG)
			_D(LOG_INFO, "cache hit  %s -> %s", src.c_str(), ret.c_str());
			#endif /* DEBUG_REGISTER_ALLOC */
		} else if (0 == _alloc_regs_.size()) {
			#if defined(DEBUG_REGISTER_ALLOC) || defined(DEBUG)
			for (auto it : _alloc_regs_map_) {
				_D(LOG_ERROR, "%s -> %s", it.first.c_str(), it.second.c_str());
			}
			#endif /* DEBUG_REGISTER_ALLOC */

			_D(LOG_CRIT, "Not implement - register burn-out");
		} else {
			ret = _alloc_regs_[0];
			_alloc_regs_map_[src] = ret;
			_alloc_regs_.erase(_alloc_regs_.begin());

			#if defined(DEBUG_REGISTER_ALLOC) || defined(DEBUG)
			_D(LOG_WARNING, "cache miss %s -> %s", src.c_str(), ret.c_str());
			#endif /* DEBUG_REGISTER_ALLOC */
		}

		blFound = false;
		for (size_t cur = irpos+1; cur < _irs_.size(); ++cur) {
			if (_irs_[cur].src == src) {
				blFound = true;
				break;
			}
		}
		if (false == blFound) {
			#if defined(DEBUG_REGISTER_ALLOC) || defined(DEBUG)
			_D(LOG_WARNING, "recycle           <- %s", ret.c_str());
			#endif /* DEBUG_REGISTER_ALLOC */
			_alloc_regs_.push_back(ret);
		}

		pos = std::find(regs.begin(), regs.end(), ret) - regs.begin();

		/* Change the size to related register */
		       if (ZASM_MEM_BYTE  == size) {
			ret = regs[(pos & 0xE0) + (pos % 8) + 0x18];
		} else if (ZASM_MEM_WORD  == size) {
			ret = regs[(pos & 0xE0) + (pos % 8) + 0x10];
		} else if (ZASM_MEM_DWORD == size) {
			ret = regs[(pos & 0xE0) + (pos % 8) + 0x08];
		} else if (ZASM_MEM_QWORD == size) {
			ret = regs[(pos & 0xE0) + (pos % 8) + 0x00];
		} else {
			_D(LOG_CRIT, "Not implement %s", size.c_str());
		}
	} else if (src == __IR_SYSCALL_REG__) {					/* reserved */
		#if defined(DEBUG_REGISTER_ALLOC) || defined(DEBUG)
		_D(LOG_WARNING, "syscall    %s -> %s", __IR_SYSCALL_REG__, SYSCALL_REG);
		#endif /* DEBUG_REGISTER_ALLOC */
		ret = SYSCALL_REG;
	}

	return ret;
}
