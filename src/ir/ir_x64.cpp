/* Copyright (C) 2014-2016 cmj. All right reserved. */
#ifdef __x86_64__

#include "zerg.h"
void IR::emit(IROP opcode, STRING _dst, STRING _src, STRING size, STRING index) {
	std::string dst, src, idx;
	std::string sys_param[] = { SYSCALL_PARAM };

	dst = IR::regalloc(_dst, size);
	src = IR::regalloc(_src, size);
	idx = IR::regalloc(index, ZASM_MEM_DWORD);
	switch(opcode) {
		/* memory access*/
			case IR_MEMORY_LOAD:
				if (__IR_REFERENCE__ == _src.substr(0, 1)) {
					/* Load from referenced data */
					(*this) += new Instruction("lea", dst, src);
				} else {
					src = localvar(_src, size, idx);
					(*this) += new Instruction("mov", dst, src);
				}
				break;
			case IR_MEMORY_STORE:
				if (__IR_REFERENCE__ == _src.substr(0, 1)) {
					/* Load from referenced data */
					(*this) += new Instruction("lea", dst, src);
				} else if (dst != src) {
					/* simple register movement */
					dst = localvar(_dst, size, idx);
					(*this) += new Instruction("mov", dst, src);
				}
				break;
			case IR_MEMORY_XCHG:
				(*this) += new Instruction("xchg", dst, src);
				break;
			case IR_MEMORY_PUSH:
				dst = localvar(_dst, size, idx);
				(*this) += new Instruction("push", dst);
				this->_syscall_nr_ ++;
				break;
			case IR_MEMORY_POP:
				this->_syscall_nr_ --;
				(*this) += new Instruction("pop", dst);
				break;
			case IR_MEMORY_PARAM:
				this->_params_.push_back(dst);
				break;
		/* arithmetic operation */
			case IR_ARITHMETIC_ADD:
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(!(IR_TOKEN_REGISTER == token(_src) || IR_TOKEN_INT == token(_src)));
				(*this) += new Instruction("add", dst, src);
				break;
			case IR_ARITHMETIC_SUB:
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(!(IR_TOKEN_REGISTER == token(_src) || IR_TOKEN_INT == token(_src)));
				(*this) += new Instruction("sub", dst, src);
				break;
			case IR_ARITHMETIC_MUL:
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(!(IR_TOKEN_REGISTER == token(_src) || IR_TOKEN_INT == token(_src)));
				(*this) += new Instruction("mul", dst, src);
				break;
			case IR_ARITHMETIC_DIV:
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(!(IR_TOKEN_REGISTER == token(_src) || IR_TOKEN_INT == token(_src)));
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(!(IR_TOKEN_REGISTER == token(_src) || IR_TOKEN_INT == token(_src)));
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
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(!(IR_TOKEN_REGISTER == token(_src) || IR_TOKEN_INT == token(_src)));
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(!(IR_TOKEN_REGISTER == token(_src) || IR_TOKEN_INT == token(_src)));
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
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(!(IR_TOKEN_REGISTER == token(_src) || IR_TOKEN_INT == token(_src)));

				if ("rcx" == regalloc(_dst, ZASM_MEM_QWORD)) {
					/* save the rax register */
					(*this) += new Instruction("push", "rax");
					(*this) += new Instruction("mov", "rax", regalloc(_dst, ZASM_MEM_QWORD));
					dst = "rax";
				} else {
					dst = regalloc(_dst, size);
				}

				if ("rcx" != regalloc(_src, ZASM_MEM_QWORD)) {
					(*this) += new Instruction("push", "rcx");
					(*this) += new Instruction("mov", "rcx", regalloc(_src, ZASM_MEM_QWORD));
					src = "cl";
				} else {
					src = regalloc(_src, ZASM_MEM_BYTE);
				}

				(*this) += new Instruction("shr", dst, src);

				if ("rcx" != regalloc(_src, ZASM_MEM_QWORD)) {
					(*this) += new Instruction("pop", "rcx");
				}
				if ("rcx" == regalloc(_dst, ZASM_MEM_QWORD)) {
					/* save the rax register */
					(*this) += new Instruction("mov", regalloc(_dst, ZASM_MEM_QWORD), "rax");
					(*this) += new Instruction("pop", "rax");
				}
				break;
			case IR_ARITHMETIC_SHL:
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(!(IR_TOKEN_REGISTER == token(_src) || IR_TOKEN_INT == token(_src)));

				if ("rcx" == regalloc(_dst, ZASM_MEM_QWORD)) {
					/* save the rax register */
					(*this) += new Instruction("push", "rax");
					(*this) += new Instruction("mov", "rax", regalloc(_dst, ZASM_MEM_QWORD));
					dst = "rax";
				} else {
					dst = regalloc(_dst, size);
				}

				if ("rcx" != regalloc(_src, ZASM_MEM_QWORD)) {
					(*this) += new Instruction("push", "rcx");
					(*this) += new Instruction("mov", "rcx", regalloc(_src, ZASM_MEM_QWORD));
					src = "cl";
				} else {
					src = regalloc(_src, ZASM_MEM_BYTE);
				}

				(*this) += new Instruction("shl", dst, src);

				if ("rcx" != regalloc(_src, ZASM_MEM_QWORD)) {
					(*this) += new Instruction("pop", "rcx");
				}
				if ("rcx" == regalloc(_dst, ZASM_MEM_QWORD)) {
					/* save the rax register */
					(*this) += new Instruction("mov", regalloc(_dst, ZASM_MEM_QWORD), "rax");
					(*this) += new Instruction("pop", "rax");
				}
				break;
			case IR_ARITHMETIC_INC:
				ALERT(IR_TOKEN_REGISTER != token(_dst) || IR_TOKEN_REGISTER != token(_src));
				(*this) += new Instruction("inc", dst);
				break;
			case IR_ARITHMETIC_DEC:
				ALERT(IR_TOKEN_REGISTER != token(_dst) || IR_TOKEN_REGISTER != token(_src));
				(*this) += new Instruction("inc", dst);
				break;
		/* logical operation */
			case IR_LOGICAL_AND:
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(!(IR_TOKEN_REGISTER == token(_src) || IR_TOKEN_INT == token(_src)));
				(*this) += new Instruction("and", dst, src);
				break;
			case IR_LOGICAL_OR:
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(!(IR_TOKEN_REGISTER == token(_src) || IR_TOKEN_INT == token(_src)));
				(*this) += new Instruction("or", dst, src);
				break;
			case IR_LOGICAL_XOR:
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(!(IR_TOKEN_REGISTER == token(_src) || IR_TOKEN_INT == token(_src)));
				(*this) += new Instruction("xor", dst, src);
				break;
			case IR_LOGICAL_NOT:
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(IR_TOKEN_UNKNOWN != token(_src));
				(*this) += new Instruction("not", dst);
				break;
			case IR_LOGICAL_NEG:
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(IR_TOKEN_UNKNOWN != token(_src));
				(*this) += new Instruction("neg", dst);
				break;
			case IR_LOGICAL_EQ:
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(!(IR_TOKEN_REGISTER == token(_src) || IR_TOKEN_INT == token(_src)));
				ALERT("" == _dst && "" == _src);
				(*this) += new Instruction("cmp", dst, src);
				(*this) += new Instruction("xor", dst, dst);
				(*this) += new Instruction("setz", this->regalloc(_dst, ZASM_MEM_BYTE));
				break;
			case IR_LOGICAL_LS:
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(!(IR_TOKEN_REGISTER == token(_src) || IR_TOKEN_INT == token(_src)));
				ALERT("" == _dst && "" == _src);
				(*this) += new Instruction("cmp", dst, src);
				(*this) += new Instruction("xor", dst, dst);
				(*this) += new Instruction("setl", this->regalloc(_dst, ZASM_MEM_BYTE));
				break;
			case IR_LOGICAL_GT:
				ALERT(IR_TOKEN_REGISTER != token(_dst));
				ALERT(!(IR_TOKEN_REGISTER == token(_src) || IR_TOKEN_INT == token(_src)));
				ALERT("" == _dst && "" == _src);
				(*this) += new Instruction("cmp", dst, src);
				(*this) += new Instruction("xor", dst, dst);
				(*this) += new Instruction("setg", this->regalloc(_dst, ZASM_MEM_BYTE));
				break;
		/* condition / control flow */
			case IR_CONDITION_JMP:
				ALERT(!(IR_TOKEN_REF == token(_dst) && IR_TOKEN_UNKNOWN == token(_src)));
				(*this) += new Instruction("jmp", dst);
				break;
			case IR_CONDITION_JMPIFN:
				ALERT(!(IR_TOKEN_REF == token(_dst) && IR_TOKEN_REGISTER == token(_src)));
				(*this) += new Instruction("cmp", src, "0x0");
				(*this) += new Instruction("je", dst);
				break;
			case IR_CONDITION_CALL:
				ALERT(!(IR_TOKEN_REF == token(_dst) || IR_TOKEN_REGISTER == token(_dst)));
				ALERT(IR_TOKEN_UNKNOWN != token(_src));
				(*this) += new Instruction("call", dst);
				break;
			case IR_CONDITION_RET:
				ALERT(!(IR_TOKEN_UNKNOWN == token(_dst) || IR_TOKEN_REGISTER == token(_dst)));
				ALERT(IR_TOKEN_UNKNOWN != token(_src));
				if ("" != _dst) (*this) += new Instruction("mov", "rax", dst);
				(*this) += new Instruction("ret");
				break;
		/* extra */
			case IR_NOP:
				ALERT(!(IR_TOKEN_UNKNOWN == token(_dst) && IR_TOKEN_UNKNOWN == token(_src)));
				(*this) += new Instruction("nop");
				break;
			case IR_PROLOGUE:
				ALERT(!(IR_TOKEN_UNKNOWN == token(_dst) || IR_TOKEN_INT == token(_dst)));
				ALERT(IR_TOKEN_UNKNOWN != token(_src));

				/* FIXME - Should be more smart */
				(*this) += new Instruction("push", "rbp");
				(*this) += new Instruction("mov", "rbp", "rsp");

				if ("" != _dst) {
					(*this) += new Instruction("sub", "rsp", _dst);
				}
				break;
			case IR_EPILOGUE:
				ALERT(!(IR_TOKEN_UNKNOWN == token(_dst) || IR_TOKEN_INT == token(_dst)));
				ALERT(IR_TOKEN_UNKNOWN != token(_src));

				if ("" != _dst) {
					(*this) += new Instruction("add", "rsp", _dst);
				}
				(*this) += new Instruction("pop", "rbp");
				this->_params_.clear();
				break;
			case IR_INTERRUPT:
				ALERT(!(IR_TOKEN_UNKNOWN == token(_dst) && IR_TOKEN_UNKNOWN == token(_src)));
				ALERT(this->_syscall_nr_ > ARRAY_SIZE(sys_param));

				while (0 < this->_syscall_nr_) {
					/* save as the parameter */
					(*this) += new Instruction("pop", sys_param[--this->_syscall_nr_]);
				}
				(*this) += new Instruction("syscall");
				break;
			case IR_LABEL:
				ALERT(!(IR_TOKEN_UNKNOWN == token(_dst) || IR_TOKEN_VAR == token(_dst)));
				ALERT(IR_TOKEN_UNKNOWN != token(_src));
				ALERT("" == dst);

				(*this) += new Instruction(dst+":");
				break;
			case IR_DEFINE:
				(*this) += new Instruction("define", dst, src);
				break;
			case IR_INLINE_ASM:
				ALERT(IR_TOKEN_STRING != token(_dst) || IR_TOKEN_UNKNOWN != token(_src));
				this->assembleL(dst.substr(1, dst.size()-2));
				break;
		default:
			_D(LOG_CRIT, "Not Implemented #%d %s %s %s #%d", opcode, _dst.c_str(),
															_src.c_str(), size.c_str(),
															opcode);
			break;
	}

	this->regfree(src);
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
		} else if (ZASM_MEM_QWORD == size) {
			pos = std::find(regs.begin(), regs.end(), tmp) - regs.begin();
			tmp = regs[(pos & 0xE0) + (pos % 8)];
		}

		_alloc_regs_map_[src] = tmp;
		dst = tmp;
		goto END;
	}
END:
	if (src != dst) {
		_D(LOG_REGISTER_ALLOC, "register reallocate %8s -> %4s", src.c_str(), dst.c_str());
	}
	return dst;
}
void IR::regfree(std::string src) {
	int cnt = 0;
	std::vector<std::string> regs = { REGISTERS };

	if (regs.end() != std::find(regs.begin(), regs.end(), src)) {
		cnt = std::find(regs.begin(), regs.end(), src) - regs.begin();
		cnt = cnt % 8;
		this->_alloc_regs_.push_back(regs[cnt]);
		/* restore the used register */
		_D(LOG_REGISTER_ALLOC, "register reallocate <- %s", regs[cnt].c_str());
	}
}
std::string IR::localvar(std::string _src, std::string size, std::string idx) {
	size_t cnt = 0;
	char buff[BUFSIZ] = {0};
	const char *fmt = NULL;
	std::string src, reg;

	size = "" == size ? ZASM_MEM_QWORD : size;
	switch(IR::token(_src)) {
		case IR_TOKEN_VAR:
			 if (_params_.end() != std::find(_params_.begin(), _params_.end(), _src)) {
				cnt = std::find(_params_.begin(), _params_.end(), _src) - _params_.begin();
				cnt = cnt + 2;
				fmt = "%s [rbp+0x%lX]";
			} else if (_locals_.end() != std::find(_locals_.begin(), _locals_.end(), _src)) {
				cnt = std::find(_locals_.begin(), _locals_.end(), _src) - _locals_.begin();
				cnt = cnt + 1;
				fmt = "%s [rbp-0x%lX]";
			} else {
				IR::localvar(_src);
				cnt = std::find(_locals_.begin(), _locals_.end(), _src) - _locals_.begin();
				cnt = cnt + 1;
				fmt = "%s [rbp-0x%lX]";
			}

			if ("" == idx) {
				snprintf(buff, sizeof(buff), fmt, size.c_str(), cnt * PARAM_SIZE);
			} else {
				reg = IR::tmpreg();

				snprintf(buff, sizeof(buff), fmt, ZASM_MEM_QWORD, cnt * PARAM_SIZE);
				(*this) += new Instruction("mov", reg.c_str(), buff);
				fmt = "%s [%s+%s]";
				snprintf(buff, sizeof(buff), fmt, size.c_str(), reg.c_str(), idx.c_str());
			}
			break;
		default:
			/* others */
			src = IR::regalloc(_src, ZASM_MEM_QWORD);
			snprintf(buff, sizeof(buff), "%s", src.c_str());
			break;
	}

	_D(LOG_DEBUG_IR, "local variable %s -> %s", _src.c_str(), buff);
	return buff;
}

#endif /* __x86_64__ */
