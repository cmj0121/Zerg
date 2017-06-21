/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <stdlib.h>
#include "zerg.h"

IR::IR(std::string dst, Args &args) : Zasm(dst, args) {
	this->_param_nr_	= 0;
	this->_lineno_		= 0;
	this->_args_		= args;
}
IR::~IR(void) {
	if (!this->_args_.only_ir) {
		Zasm::dump(this->_args_.entry, this->_args_.symbol);
	}
}

void IR::compile(std::string src) {
	std::string line;
	std::fstream fs(src);

	if (!fs.is_open()) {
		/* cannot open the source code */
		_D(LOG_CRIT, "source file `%s` does NOT exist", src.c_str());
	}

	while (std::getline(fs, line)) {
		unsigned int pos, nr = 0;
		std::string irs[4] = {""};

		/* lexer IR line-by-line */
		_D(LOG_DEBUG_LEXER, "IR lexer - %s L#%d", line.c_str(), _lineno_);
		for (unsigned cur = 0; cur <= line.size(); ++cur) {
			switch(line[cur]) {
				case '\0': case '#':
					/* Need not process anymore */
					_lineno_ ++;
					goto EMIT;
				case ' ': case '\t':
					/* whitespace */
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
			if (0 != nr) IR::emit(irs[0], irs[1], irs[2], irs[3]);
	}
}
IROP IR::opcode(std::string src) {
	IROP opcode = IR_UNKNOWN;
	std::vector<std::pair<std::string, IROP>> map = {
		/* memory access*/
		{"STORE"	, IR_MEMORY_STORE},
		{"LOAD"		, IR_MEMORY_LOAD},
		{"XCHG"		, IR_MEMORY_XCHG},
		{"PUSH"		, IR_MEMORY_PUSH},
		{"POP"		, IR_MEMORY_POP},
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
		{"JMPIF"	, IR_CONDITION_JMPIF},
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

	for (auto it : map) {
		if (it.first == src) opcode = it.second;
	}

	_D(LOG_DEBUG, "%s -> #%X", src.c_str(), opcode);
	return opcode;
}
void IR::emit(std::string op, std::string _dst, std::string _src, std::string size) {
	std::string dst, src;
	std::string sys_param[] = { SYSCALL_PARAM };
	IROP opcode = IR::opcode(op);

	dst = IR::regalloc(_dst, size);
	src = IR::regalloc(_src, size);
	switch(opcode) {
		/* memory access*/
			case IR_MEMORY_LOAD:
				(*this) += new Instruction("mov", dst, src);
				break;
			case IR_MEMORY_STORE:
				if (__IR_REFERENCE__ == src.substr(0, 1)) {
					/* Load from referenced data */
					(*this) += new Instruction("lea", dst, src);
				} else if (dst != src) {
					/* simple register movement */
					(*this) += new Instruction("mov", dst, src);
				}
				break;
			case IR_MEMORY_XCHG:
				(*this) += new Instruction("xchg", dst, src);
				break;
			case IR_MEMORY_PUSH:
				(*this) += new Instruction("push", dst);
				this->_param_nr_ ++;
				break;
			case IR_MEMORY_POP:
				this->_param_nr_ --;
				(*this) += new Instruction("pop", dst);
				break;
		/* arithmetic operation */
			case IR_ARITHMETIC_ADD:
				(*this) += new Instruction("add", dst, src);
				break;
			case IR_ARITHMETIC_SUB:
				(*this) += new Instruction("sub", dst, src);
				break;
			case IR_ARITHMETIC_MUL:
				(*this) += new Instruction("mul", dst, src);
				break;
			case IR_ARITHMETIC_DIV:
				(*this) += new Instruction("push", "rdx");
				if (dst != "rax") (*this) += new Instruction("push", "rax");
				if (dst != "rax") (*this) += new Instruction("mov", "rax", dst);

				(*this) += new Instruction("xor", "rdx", "rdx");
				(*this) += new Instruction("cqo");
				(*this) += new Instruction("idiv", src);

				if (dst != "rax") (*this) += new Instruction("mov", dst, "rax");
				if (dst != "rax") (*this) += new Instruction("pop", "rax");
				(*this) += new Instruction("pop", "rdx");
				break;
			case IR_ARITHMETIC_MOD:
				/* FIXME - Should always return the sign same as divisor */
				(*this) += new Instruction("push", "rdx");
				(*this) += new Instruction("push", "rax");
				if (dst != "rax") (*this) += new Instruction("mov", "rax", dst);

				(*this) += new Instruction("xor", "rdx", "rdx");
				(*this) += new Instruction("cqo");
				(*this) += new Instruction("idiv", src);

				(*this) += new Instruction("mov", dst, "rdx");
				(*this) += new Instruction("pop", "rax");
				(*this) += new Instruction("pop", "rdx");
				break;
			case IR_ARITHMETIC_SHR:
				_D(LOG_CRIT, "Not Improvement #%X", opcode);
				break;
			case IR_ARITHMETIC_SHL:
				_D(LOG_CRIT, "Not Improvement #%X", opcode);
				break;
			case IR_ARITHMETIC_INC:
				(*this) += new Instruction("inc", dst);
				break;
			case IR_ARITHMETIC_DEC:
				(*this) += new Instruction("inc", dst);
				break;
		/* logical operation */
			case IR_LOGICAL_AND:
				(*this) += new Instruction("and", dst, src);
				break;
			case IR_LOGICAL_OR:
				(*this) += new Instruction("or", dst, src);
				break;
			case IR_LOGICAL_XOR:
				(*this) += new Instruction("xor", dst, src);
				break;
			case IR_LOGICAL_NOT:
				(*this) += new Instruction("not", dst);
				break;
			case IR_LOGICAL_NEG:
				(*this) += new Instruction("neg", dst);
				break;
			case IR_LOGICAL_EQ:
				_D(LOG_CRIT, "Not Improvement #%X", opcode);
				break;
			case IR_LOGICAL_LS:
				_D(LOG_CRIT, "Not Improvement #%X", opcode);
				break;
			case IR_LOGICAL_GT:
				_D(LOG_CRIT, "Not Improvement #%X", opcode);
				break;
		/* condition / control flow */
			case IR_CONDITION_JMP:
				(*this) += new Instruction("jmp", dst);
				break;
			case IR_CONDITION_JMPIF:
				(*this) += new Instruction("cmp", src, "0x0");
				(*this) += new Instruction("jne", dst);
				break;
			case IR_CONDITION_CALL:
				(*this) += new Instruction("call", dst);
				break;
			case IR_CONDITION_RET:
				ALERT("" !=dst);
				(*this) += new Instruction("ret");
				break;
		/* extra */
			case IR_NOP:
				(*this) += new Instruction("nop");
				break;
			case IR_PROLOGUE:
				_D(LOG_CRIT, "Not Improvement #%X", opcode);
				break;
			case IR_EPILOGUE:
				_D(LOG_CRIT, "Not Improvement #%X", opcode);
				break;
			case IR_INTERRUPT:
				ALERT(this->_param_nr_ > ARRAY_SIZE(sys_param));

				while (0 < this->_param_nr_) {
					/* save as the parameter */
					(*this) += new Instruction("pop", sys_param[--this->_param_nr_]);
				}
				(*this) += new Instruction("syscall");
				break;
			case IR_LABEL:
				(*this) += new Instruction(dst+":");
				break;
			case IR_DEFINE:
				(*this) += new Instruction("define", dst, src);
				break;
			case IR_INLINE_ASM:
				_D(LOG_CRIT, "Not Improvement #%X", opcode);
				break;
		default:
			_D(LOG_CRIT, "Not Improvement %s %s %s %s #%d", op.c_str(), _dst.c_str(),
															_src.c_str(), size.c_str(),
															opcode);
			break;
	}
}
std::string IR::regalloc(std::string src, std::string size) {
	int cnt = 0;
	std::string tmp, dst = src;

	if (src == __IR_SYSCALL_REG__) {
		dst = SYSCALL_REG;
		goto END;
	} else if (1 ==  sscanf(src.c_str(), __IR_REG_FMT__, &cnt)) {
		int pos = 0;
		std::vector<std::string> regs = { REGISTERS };

		if (0 != _alloc_regs_map_.count(src)) {
			/* HACK - Found in cache */
			tmp = _alloc_regs_map_[src];
		} else {
			ALERT(0 == _alloc_regs_.size());

			tmp = _alloc_regs_[0];
			_alloc_regs_.erase(_alloc_regs_.begin());
		}

		/* HACK - resize the register if need */
		if (ZASM_MEM_BYTE         == size) {
			pos = std::find(regs.begin(), regs.end(), tmp) - regs.begin();
			tmp = regs[(pos & 0xE0) + (pos % 8) + 24];
		} else if (ZASM_MEM_WORD  == size) {
			pos = std::find(regs.begin(), regs.end(), tmp) - regs.begin();
			tmp = regs[(pos & 0xE0) + (pos % 8) + 16];
		} else if (ZASM_MEM_DWORD == size) {
			pos = std::find(regs.begin(), regs.end(), tmp) - regs.begin();
			tmp = regs[(pos & 0xE0) + (pos % 8) + 8];
		}

		_alloc_regs_map_[src] = tmp;
		dst = tmp;
		goto END;
	}

	_D(LOG_REGISTER_ALLOC, "register reallocate %8s -> %4s", src.c_str(), dst.c_str());
END:
	return dst;
}
std::string IR::randstr(unsigned int size, std::string prefix) {
	/* generated a random label string */
	const char CH_POOL[] = {	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
								'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
								'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
								'U', 'V', 'W', 'X', 'Y', 'Z'};
	std::string ret = prefix;

	ALERT(ret.size() >  size);
	srand(time(NULL));
	while (ret.size() < size) {
		ret = ret + CH_POOL[random() % ARRAY_SIZE(CH_POOL)];
	}

	return ret;
}