/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include "zerg.h"


void IR::emit_x64(size_t irpos) {
	char buff[BUFSIZ] = {0};
	std::string sys_param[] = { SYSCALL_PARAM };
	std::string dst = _irs_[irpos].dst,
				src = _irs_[irpos].src,
				siz = _irs_[irpos].size,
				idx = _irs_[irpos].index;

	switch(_irs_[irpos].opcode) {
		/* memory access*/
			case IR_MEMORY_LOAD:
				if (__IR_REFERENCE__ == src.substr(0, 1)) {
					/* Load from referenced data */
					(*this) += new Instruction("lea", dst, src);
				} else if ("" != idx && IR::is_register(src)) {
					snprintf(buff, sizeof(buff), "%s [%s+%s]",
									siz.c_str(), src.c_str(), idx.c_str());
					(*this) += new Instruction("mov", dst, buff);
				} else if ("" != idx) {
					std::string tmpreg;

					tmpreg = IR::regalloc(-1, __IR_REG_TMP__);
					snprintf(buff, sizeof(buff), "%s [%s+%s]",
										siz.c_str(), tmpreg.c_str(), idx.c_str());

					(*this) += new Instruction("mov", tmpreg, IR::variable(src));
					(*this) += new Instruction("mov", dst, buff);
				} else if (dst != src){
					/* simple register movement */
					(*this) += new Instruction("mov", dst, IR::variable(src));
				}
				break;
			case IR_MEMORY_STORE:
				if (__IR_REFERENCE__ == src.substr(0, 1)) {
					/* Load from referenced data */
					(*this) += new Instruction("lea", dst, src);
				} else if ("" != idx && IR::is_register(dst)) {
					snprintf(buff, sizeof(buff), "%s [%s+%s]",
									siz.c_str(), dst.c_str(), idx.c_str());
					(*this) += new Instruction("mov", buff, src);
				} else if ("" != idx) {
					std::string tmpreg;

					tmpreg = IR::regalloc(-1, __IR_REG_TMP__);
					snprintf(buff, sizeof(buff), "%s [%s+%s]",
										siz.c_str(), tmpreg.c_str(), idx.c_str());

					(*this) += new Instruction("mov", tmpreg, IR::variable(dst));
					(*this) += new Instruction("mov", buff, src);
				} else if (dst != src){
					/* simple register movement */
					(*this) += new Instruction("mov", IR::variable(dst), src);
				}
				break;
			case IR_MEMORY_XCHG:
				(*this) += new Instruction("xchg", IR::variable(dst), src);
				break;
			case IR_MEMORY_PUSH:
				(*this) += new Instruction("push", IR::variable(dst));
				this->_syscall_nr_ ++;
				break;
			case IR_MEMORY_POP:
				this->_syscall_nr_ --;
				(*this) += new Instruction("pop", IR::variable(dst));
				break;
			case IR_MEMORY_PARAM:
				this->_params_.push_back(dst);
				break;
		/* arithmetic operation */
			case IR_ARITHMETIC_ADD:
				(*this) += new Instruction("add", IR::variable(dst), src);
				break;
			case IR_ARITHMETIC_SUB:
				(*this) += new Instruction("sub", IR::variable(dst), src);
				break;
			case IR_ARITHMETIC_MUL:
				(*this) += new Instruction("mul", IR::variable(dst), src);
				break;
			case IR_ARITHMETIC_DIV:
				/* FIXME - Should NOT use div in assembly
				 *
				 * By the link [0], div (36 uops) is more slower than shr (2 uops)
				 *
				 * [0] https://stackoverflow.com/questions/40354978
				 */
				_D(LOG_CRIT, "Not Implemented div");
				break;
			case IR_ARITHMETIC_MOD:
				/* FIXME - Should always return the sign same as divisor */
				(*this) += new Instruction("push", "rdx");
				(*this) += new Instruction("push", "rax");
				if (dst != "rax") {
					(*this) += new Instruction("mov", "rax", IR::variable(dst));
				}

				(*this) += new Instruction("xor", "rdx", "rdx");
				(*this) += new Instruction("cqo");
				(*this) += new Instruction("idiv", src);

				(*this) += new Instruction("mov", IR::variable(dst), "rdx");
				(*this) += new Instruction("pop", "rax");
				(*this) += new Instruction("pop", "rdx");
				break;
			case IR_ARITHMETIC_SHR:
			case IR_ARITHMETIC_SHL:
				_D(LOG_CRIT, "Not Implemented");
			case IR_ARITHMETIC_INC:
				(*this) += new Instruction("inc", dst);
				break;
			case IR_ARITHMETIC_DEC:
				(*this) += new Instruction("dec", dst);
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
				(*this) += new Instruction("cmp", dst, src);
				(*this) += new Instruction("setz", dst);
				break;
			case IR_LOGICAL_LS:
				(*this) += new Instruction("cmp", dst, src);
				(*this) += new Instruction("setl", dst);
				break;
			case IR_LOGICAL_GT:
				(*this) += new Instruction("cmp", dst, src);
				(*this) += new Instruction("setg", dst);
				break;
		/* condition / control flow */
			case IR_CONDITION_JMP:
				(*this) += new Instruction("jmp", dst);
				break;
			case IR_CONDITION_JMPIFN:
				(*this) += new Instruction("cmp", src, "0x0");
				(*this) += new Instruction("je", dst);
				break;
			case IR_CONDITION_CALL:
				(*this) += new Instruction("call", dst);
				break;
			case IR_CONDITION_RET:
				if ("" != dst) {
					src = IR::regalloc(-1, __IR_SYSCALL_REG__);
					(*this) += new Instruction("mov", src, dst);
				}
				(*this) += new Instruction("ret");
				break;
		/* extra */
			case IR_NOP:
				(*this) += new Instruction("nop");
				break;
			case IR_PROLOGUE:
				/* reset all register */
				this->_alloc_regs_ = { USED_REGISTERS };

				/* FIXME - Should be more smart */
				(*this) += new Instruction("push", "rbp");
				(*this) += new Instruction("mov", "rbp", "rsp");

				(*this) += new Instruction("sub", "rsp", dst);
				break;
			case IR_EPILOGUE:
				(*this) += new Instruction("add", "rsp", dst);
				(*this) += new Instruction("pop", "rbp");
				this->_params_.clear();
				break;
			case IR_INTERRUPT:
				while (0 < this->_syscall_nr_) {
					/* save as the parameter */
					(*this) += new Instruction("pop", sys_param[--this->_syscall_nr_]);
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
			_D(LOG_CRIT, "Not Implemented #%d %s %s %s %s",
				_irs_[irpos].opcode, dst.c_str(), src.c_str(), siz.c_str(), idx.c_str());
			break;
	}
}

