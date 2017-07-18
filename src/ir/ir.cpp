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
		std::string irs[4] = {""}, tmp;

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
			if (0 != nr) IR::emit(irs[0], irs[1], irs[2], irs[3]);
	}
}
IROP IR::opcode(std::string src) {
	IROP opcode = IR_UNKNOWN;

	for (auto it : IROP_map) {
		if (it.first == src) opcode = it.second;
	}

	_D(LOG_DEBUG, "%s -> #%X", src.c_str(), opcode);
	return opcode;
}
void IR::emit(std::string op, std::string _dst, std::string _src, std::string size) {
	IROP opcode = IR::opcode(op);

	_D(LOG_DEBUG_IR, "emit %s %s %s %s", op.c_str(), _dst.c_str(),
											_src.c_str(), size.c_str());
	this->emit(opcode, _dst, _src, size);
}
void IR::emit(IROP opcode, std::string _dst, std::string _src, std::string size) {
	std::string dst, src;
	std::string sys_param[] = { SYSCALL_PARAM };

	dst = IR::regalloc(_dst, size);
	src = IR::regalloc(_src, size);
	switch(opcode) {
		/* memory access*/
			case IR_MEMORY_LOAD:
				if ('.' != src[0]) {
					size_t cnt = 0;
					char buff[BUFSIZ] = {0};

					if (_local_.end() != std::find(_local_.begin(), _local_.end(), src)) {
						cnt = std::find(_local_.begin(), _local_.end(), src) - _local_.begin();
						snprintf(buff, sizeof(buff), "[rbp-0x%lX]", (cnt+1)* PARAM_SIZE);
						(*this) += new Instruction("mov", dst, buff);
					} else if (__IR_REFERENCE__ == src.substr(0, 1)) {
						(*this) += new Instruction("lea", dst, src);
					} else {
						(*this) += new Instruction("mov", dst, src);
					}
				} else {
					(*this) += new Instruction("mov", dst, src);
				}
				break;
			case IR_MEMORY_STORE:
				if (__IR_REFERENCE__ == src.substr(0, 1)) {
					/* Load from referenced data */
					(*this) += new Instruction("lea", dst, src);
				} else if ('.' != dst[0]) {
					size_t cnt = 0;
					char buff[BUFSIZ] = {0};

					if (_local_.end() == std::find(_local_.begin(), _local_.end(), dst)) {
						/* new token, treated as local variable */
						cnt = _local_.size();
						snprintf(buff, sizeof(buff), "[rbp-0x%lX]", (cnt+1) * PARAM_SIZE);
						(*this) += new Instruction("mov", buff, src);

						_local_.push_back(dst);
					} else {
						cnt = std::find(_local_.begin(), _local_.end(), dst) - _local_.begin();
						snprintf(buff, sizeof(buff), "[rbp-0x%lX]", (cnt+1) * PARAM_SIZE);
						(*this) += new Instruction("mov", buff, src);
					}
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
				_D(LOG_CRIT, "Not Implemented #%X", opcode);
				break;
			case IR_ARITHMETIC_SHL:
				_D(LOG_CRIT, "Not Implemented #%X", opcode);
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
				_D(LOG_CRIT, "Not Implemented #%X", opcode);
				break;
			case IR_LOGICAL_LS:
				_D(LOG_CRIT, "Not Implemented #%X", opcode);
				break;
			case IR_LOGICAL_GT:
				_D(LOG_CRIT, "Not Implemented #%X", opcode);
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
				if ("" != _dst) (*this) += new Instruction("mov", "rax", dst);
				(*this) += new Instruction("ret");
				break;
		/* extra */
			case IR_NOP:
				(*this) += new Instruction("nop");
				break;
			case IR_PROLOGUE:
				/* FIXME - Should be more smart */
				(*this) += new Instruction("push", "rcx");
				(*this) += new Instruction("push", "rdx");
				(*this) += new Instruction("push", "rbx");

				(*this) += new Instruction("push", "rbp");
				(*this) += new Instruction("mov", "rbp", "rsp");
				if ("" != _dst) {
					(*this) += new Instruction("sub", "rsp", _dst);
				}
				break;
			case IR_EPILOGUE:
				if ("" != _dst) {
					(*this) += new Instruction("add", "rsp", _dst);
				}
				(*this) += new Instruction("pop", "rbp");

				/* FIXME - Should be more smart */
				(*this) += new Instruction("pop", "rbx");
				(*this) += new Instruction("pop", "rdx");
				(*this) += new Instruction("pop", "rcx");
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
				this->assembleL(dst.substr(1, dst.size()-2));
				break;
		default:
			_D(LOG_CRIT, "Not Implemented #%d %s %s %s #%d", opcode, _dst.c_str(),
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
