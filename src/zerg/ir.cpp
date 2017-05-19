/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <stdlib.h>
#include "zerg.h"

IR::IR(std::string dst, ZergArgs *args) : Binary(dst, args->_pie_) {
	this->_args_ = args;
}
IR::~IR(void) {
	if (!_args_->_only_ir_) {
		Binary::dump(_args_->_entry_, _args_->_symbol_);
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
		std::string irs[5] = {""};


		/* lexer IR line-by-line */
		_D(LOG_DEBUG, "IR lexer - %s L#%d", line.c_str(), _lineno_);
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
					irs[nr++] = line.substr(cur, pos-cur);
					cur       = pos;
					break;
			}
		}

		EMIT:
			if (0 != nr) IR::emit(irs[0], irs[1], irs[2], irs[3], irs[4]);
	}
}
IROP IR::opcode(std::string src) {
	IROP opcode = IROP_UNKNOWN;
	std::vector<std::pair<std::string, IROP>> map = {
		{"XCHG"		, IROP_XCHG},
		{"STORE"	, IROP_STORE},
		{"LOAD"		, IROP_LOAD},

		{"ADD"		, IROP_ADD},
		{"SUB"		, IROP_SUB},
		{"MUL"		, IROP_MUL},
		{"DIV"		, IROP_DIV},
		{"REM"		, IROP_REM},
		{"INC"		, IROP_INC},
		{"DEC"		, IROP_DEC},
		{"SHL"		, IROP_SHL},
		{"SHR"		, IROP_SHR},

		{"AND"		, IROP_AND},
		{"OR"		, IROP_OR},
		{"XOR"		, IROP_XOR},
		{"NOT"		, IROP_NOT},
		{"NEG"		, IROP_NEG},
		{"EQ"		, IROP_EQ},
		{"LS"		, IROP_LS},
		{"LE"		, IROP_LE},
		{"GE"		, IROP_GE},
		{"GT"		, IROP_GT},

		{"JMP"		, IROP_JMP},
		{"JMPIF"	, IROP_JMPIF},

		{"CALL"		, IROP_CALL},
		{"RET"		, IROP_RET},
		{"PARAM"	, IROP_PARAM},
		{"PROLOGUE"	, IROP_PROLOGUE},
		{"EPILOGUE"	, IROP_EPILOGUE},

		{"INTERRUPT", IROP_INTERRUPT},
		{"NOP"		, IROP_NOP},
		{"LABEL"	, IROP_LABEL},
		{"ASM"		, IROP_ASM},
		{"->"		, IROP_ASM},
	};

	for (auto it : map) {
		if (it.first == src) opcode = it.second;
	}

	return opcode;
}
void IR::emit(STRING op, STRING _dst, STRING _src, STRING _idx, STRING _size) {
	unsigned int pos = 0, nr = 0;
	char buff[BUFSIZ] = {0};
	IROP opcode = IR::opcode(op);
	std::string src = "", dst = "", idx = "", tmp = "";
	std::vector<std::string> regs = { REGISTERS };
	std::vector<std::string> sysregs = { "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9"};



	dst = this->regalloc(_dst, _size);
	src = __IR_DUMMY__ != _idx ? this->regalloc(_src) : this->regalloc(_src, _size);
	idx = this->regalloc(_idx);

	_D(LOG_INFO, "IR emit - %s %s %s %s %s",
			op.c_str(), dst.c_str(), src.c_str(), idx.c_str(), _size.c_str());

	switch(opcode) {
		/* data-related */
		case IROP_XCHG:				/* (XCHG,  DST, SRC) */
			ALERT("" == dst || "" == src);
			_D(LOG_CRIT, "Not Implemented");
			break;
		case IROP_STORE:			/* (STORE, DST, SRC, IDX, SIZE) */
			/* Load data from memory with index if need */
			/* Reference */
			if (src[0] == __IR_REFERENCE__[0]) {
				std::string tmp = this->tmpreg();

				(*this) += new Instruction("lea", tmp, src);
				src = tmp;
			}

			if (dst == _dst) {						/* save variable */
				pos = std::find(_stack_.begin(), _stack_.end(), dst) - _stack_.begin();
				std::string tmpreg = this->tmpreg();

				/* HACK - only allow created variable */
				ALERT(_stack_.size() == pos && ("" != idx && src != __IR_FUNC_STACK__));
				if (_stack_.size() == pos) {
					/* save into _stack_ */
					_stack_.push_back(dst);
				}

				snprintf(buff, sizeof(buff), "[rbp-0X%X]", (pos+1) * 0x08);

				if (src == __IR_FUNC_STACK__ && "" != idx) {
					/* save the parameter into local variable */
					(*this) += new Instruction("mov", tmpreg, idx);
					(*this) += new Instruction("mov", buff, tmpreg);
				} else if ("" == idx) {
					/* save in local variable*/
					(*this) += new Instruction("mov", buff, src);
				} else {
					(*this) += new Instruction("mov", tmpreg, buff);
					snprintf(buff, sizeof(buff), "%s[%s+%s]",
									"" == _size ? "" : (_size + " ").c_str(),
									tmpreg.c_str(), idx.c_str());
					(*this) += new Instruction("mov", buff, src);
				}
			} else if (dst != src || "" != idx) {	/* register STORE */
				if ("" != idx) {
					snprintf(buff, sizeof(buff), "%s[%s+%s]",
									"" == _size ? "" : (_size + " ").c_str(),
									dst.c_str(),
									idx.c_str());
					(*this) += new Instruction("mov", buff, src);
				} else {
					(*this) += new Instruction("mov", dst, src);
				}
			} else {
				_D(LOG_ERROR, "Need NOT assemble mov `%s` `%s`", dst.c_str(), src.c_str());
			}
			break;
		case IROP_LOAD:				/* (LOAD,  DST, SRC, IDX, SIZE) */
			/* Load data from memory with index if need */
			if (__IR_FUNC_STACK__ == src) {			/* function parameter */
				ALERT(idx == "" || _size != ZASM_MEM_QWORD);

				snprintf(buff, sizeof(buff), "%s [rbp+%s]", ZASM_MEM_QWORD, idx.c_str());
				(*this) += new Instruction("mov", dst.data(), buff);
			} else if (src == _src) {				/* save local variable */
				pos = std::find(_stack_.begin(), _stack_.end(), src) - _stack_.begin();

				if (_stack_.size() == pos) {
					/* save into _stack_ */
					_D(LOG_CRIT, "Local VAR `%s` not declare", src.c_str());
				}

				if ("" == idx) {
					snprintf(buff, sizeof(buff), "[rbp-0X%X]", (pos+1) * 0x08);
					(*this) += new Instruction("mov", dst, buff);
				} else {
					std::string tmp = this->tmpreg();

					snprintf(buff, sizeof(buff), "[rbp-0X%X]", (pos+1) * 0x08);
					(*this) += new Instruction("mov", tmp.c_str(), buff);

					snprintf(buff, sizeof(buff), "%s[%s%s%s]",
									"" == _size ? "" : (_size + " ").c_str(),
									tmp.c_str(),
									'-' == idx[0] ? "" : "+",
									idx.c_str());
					if ("" != _size && ZASM_MEM_QWORD != _size) {
						/* clean-up the buffer */
						(*this) += new Instruction("xor", dst, dst);
					}
					(*this) += new Instruction("mov", dst, buff);
				}
			} else if (dst != src || "" != idx) {	/* register STORE */
				if ("" != idx) {
					snprintf(buff, sizeof(buff), "%s[%s+%s]",
									"" == _size ? "" : (_size + " ").c_str(),
									src.c_str(),
									idx.c_str());
					(*this) += new Instruction("mov", dst, buff);
				} else {
					(*this) += new Instruction("mov", dst, src);
				}
			} else {
				_D(LOG_ERROR, "Need NOT assemble mov `%s` `%s`", dst.c_str(), src.c_str());
			}
			break;

		/* arithmetic */
		case IROP_ADD:				/* (ADD,   DST, SRC) */
			/* dst = dst + src */
			(*this) += new Instruction("add", dst, src);
			break;
		case IROP_SUB: 			/* (SUB,   DST, SRC) */
			/* dst = dst - src */
			(*this) += new Instruction("sub", dst, src);
			break;
		case IROP_MUL:			/* (MUL,   DST, SRC) */
			/* dst = dst * src */
			(*this) += new Instruction("mul", dst, src);
			break;
		case IROP_DIV:			/* (DIV,   DST, SRC) */
			/* dst = dst / src */
			(*this) += new Instruction("push", "rdx");
			(*this) += new Instruction("push", "rax");

			if (dst != "rax") (*this) += new Instruction("mov", "rax", dst);
			(*this) += new Instruction("xor", "rdx", "rdx");
			(*this) += new Instruction("cqo");
			(*this) += new Instruction("idiv", src);
			(*this) += new Instruction("mov", dst, "rax");

			(*this) += new Instruction("pop", "rax");
			(*this) += new Instruction("pop", "rdx");
			break;
		case IROP_REM:			/* (REM,   DST, SRC) */
			/* dst = dst / src */
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
		case IROP_INC:			/* (ADD,   DST) */
			/* dst = dst + 1 */
			(*this) += new Instruction("inc", dst);
			break;
		case IROP_DEC:			/* (DEC,   DST) */
			/* dst = dst - 1 */
			(*this) += new Instruction("dec", dst);
			break;
		case IROP_SHL:			/* (SHL,   DST, SRC) */
			/* dst = dst << src */
			if ("rcx" == dst) {
				(*this) += new Instruction("xchg", dst, src);
				(*this) += new Instruction("sal", src, "cl");
				(*this) += new Instruction("xchg", dst, src);
			} else {
				(*this) += new Instruction("push", "rcx");
				(*this) += new Instruction("mov", "rcx", src);
				(*this) += new Instruction("sal", dst, "cl");
				(*this) += new Instruction("pop", "rcx");
			}
			break;
		case IROP_SHR:			/* (SHR,   DST, SRC) */
			/* dst = dst >> src */
			if ("rcx" == dst) {
				(*this) += new Instruction("xchg", dst, src);
				(*this) += new Instruction("sar", src, "cl");
				(*this) += new Instruction("xchg", dst, src);
			} else {
				(*this) += new Instruction("push", "rcx");
				(*this) += new Instruction("mov", "rcx", src);
				(*this) += new Instruction("sar", dst, "cl");
				(*this) += new Instruction("pop", "rcx");
			}
			break;

		/* logical */
		case IROP_AND:			/* (AND,   DST, SRC) */
			/* dst = dst & src */
			(*this) += new Instruction("and", dst, src);
			break;
		case IROP_OR:			/* (OR,    DST, SRC) */
			/* dst = dst | src */
			(*this) += new Instruction("or", dst, src);
			break;
		case IROP_XOR:			/* (XOR,   DST, SRC) */
			/* dst = dst ^ src */
			(*this) += new Instruction("xor", dst, src);
			break;
		case IROP_NOT:			/* (NOT,   DST) */
			/* negative dst with 1's complement */
			(*this) += new Instruction("not", dst);
			break;
		case IROP_NEG:			/* (NEG,   DST) */
			/* negative dst with 2's complement */
			(*this) += new Instruction("neg", dst);
			break;
		case IROP_EQ:			/* (EQ,    DST, SRC) */
			/* dst = dst eq src */
			pos = std::find(regs.begin(), regs.end(), dst) - regs.begin();

			ALERT(pos == regs.size());
			tmp = pos >= 32 ? regs[pos%8 + (8*8)] : regs[pos%8 + (8*3)];

			(*this) += new Instruction("cmp", dst, src);
			(*this) += new Instruction("setz", tmp);
			(*this) += new Instruction("and",  tmp, "0x1");
			break;
		case IROP_LS:			/* (LS,    DST, SRC) */
			/* dst = dst < src */
			pos = std::find(regs.begin(), regs.end(), dst) - regs.begin();

			ALERT(pos == regs.size());
			tmp = pos >= 32 ? regs[pos%8 + (8*8)] : regs[pos%8 + (8*3)];

			(*this) += new Instruction("cmp", dst, src);
			(*this) += new Instruction("setl", tmp);
			(*this) += new Instruction("and",  tmp, "0x1");
			break;
		case IROP_LE:			/* (LE,    DST, SRC) */
			/* dst = dst <= src */
			pos = std::find(regs.begin(), regs.end(), dst) - regs.begin();

			ALERT(pos == regs.size());
			tmp = pos >= 32 ? regs[pos%8 + (8*8)] : regs[pos%8 + (8*3)];

			(*this) += new Instruction("cmp", dst, src);
			(*this) += new Instruction("setle", tmp);
			(*this) += new Instruction("and",   tmp, "0x1");
			break;
		case IROP_GE:			/* (GE,    DST, SRC) */
			/* dst = dst >= src */
			pos = std::find(regs.begin(), regs.end(), dst) - regs.begin();

			ALERT(pos == regs.size());
			tmp = pos >= 32 ? regs[pos%8 + (8*8)] : regs[pos%8 + (8*3)];

			(*this) += new Instruction("cmp", dst, src);
			(*this) += new Instruction("setge", tmp);
			(*this) += new Instruction("and",   tmp, "0x1");
			break;
		case IROP_GT:			/* (EQ,    DST, SRC) */
			/* dst = dst > src */
			pos = std::find(regs.begin(), regs.end(), dst) - regs.begin();

			ALERT(pos == regs.size());
			tmp = pos >= 32 ? regs[pos%8 + (8*8)] : regs[pos%8 + (8*3)];

			(*this) += new Instruction("cmp", dst, src);
			(*this) += new Instruction("setg", tmp);
			(*this) += new Instruction("and",  tmp, "0x1");
			break;

		/* control flow */
		case IROP_JMP:			/* (JMP,   DST) */
			/* directly jump */
			(*this) += new Instruction("jmp", __IR_REFERENCE__ + dst);
			break;
		case IROP_JMPIF:		/* (JMPIF, DST, SRC) */
			/* jump if true */
			ALERT("" == dst || "" == src);
			(*this) += new Instruction("cmp", src, "0x0");
			(*this) += new Instruction("je", __IR_REFERENCE__ + dst);
			break;

		/* subroutine */
		case IROP_CALL:			/* (CALL,  DST, SRC) */
			/* call produce */
			nr = atoi(src.c_str());
			(*this) += new Instruction("call", __IR_REFERENCE__ + dst);

			if (0 != this->_param_nr_) {
				snprintf(buff, sizeof(buff), "0x%X", nr * 0x08);
				(*this) += new Instruction("add", "rsp", buff);
				this->_param_nr_ -= nr;
			}
			break;
		case IROP_RET:			/* (RET,   DST) */
			/* 	return from procedure */
			if (""  != dst) {
				(*this) += new Instruction("mov", SYSCALL_REG, dst);
			}

			(*this) += new Instruction("ret");
			break;
		case IROP_PARAM:			/* (PARAM, DST) */
			/* Save the parameter */
			pos = std::find(_stack_.begin(), _stack_.end(), dst) - _stack_.begin();

			if (pos == _stack_.size()) {
				/* save dst into local stack*/
				(*this) += new Instruction("push", dst);
			} else {
				snprintf(buff, sizeof(buff), "[rbp-0X%X]", (pos+1) * 0x08);
				(*this) += new Instruction("push", buff);
			}

			this->_param_nr_ ++;
			this->regsave(dst);
			break;
		case IROP_PROLOGUE:		/* (PROLOGUE, NR) */
			this->_stack_.clear();
			this->resetreg();
			(*this) += new Instruction("push", "rbp");
			(*this) += new Instruction("mov", "rbp", "rsp");
			if ("" != dst) {
				/* save the number of local variable */
				(*this) += new Instruction("sub", "rsp", dst);
			}

			this->_param_nr_ = 0;
			break;
		case IROP_EPILOGUE:		/* (EPILOGUE, NR) */
			if ("" != dst) {
				/* save the number of local variable */
				(*this) += new Instruction("add", "rsp", dst);
			}
			(*this) += new Instruction("pop", "rbp");
			break;

		/* extra */
		case IROP_INTERRUPT:	/* (INTERRUPT) */
			/* Call system interrupt, and it is platform-dependent */
			while (0 != this->_param_nr_) {
				(*this) += new Instruction("pop", sysregs[--this->_param_nr_]);
			}
			(*this) += new Instruction("syscall");
			this->_param_nr_ = 0;
			break;
		case IROP_NOP:			/* (NOP) */
			/* NOP */
			(*this) += new Instruction("nop");
			break;
		case IROP_LABEL:			/* (LABEL, DST, SRC) */
			/* Set label or set variable */
			if ("" == src) {
				(*this) += new Instruction("asm", dst + ":");
			} else {
				/* NOTE - the string need to be a "STRING" */
				(*this) += new Instruction("asm", dst, "\"" + src + "\"");
			}
			break;
		case IROP_ASM:
			(*this) += new Instruction(dst, src, idx);
			break;
		default:
			_D(LOG_CRIT, "Not Implemented opcode `%s` 0x%X", op.c_str(), opcode);
			break;
	}

	this->regsave(src);
	this->regsave(idx);
}
std::string IR::randstr(unsigned int size, std::string prefix) {
	/* generated a random label string */
	std::string ret = prefix;
	char CH_POOL[] = {	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
						'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
						'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
						'U', 'V', 'W', 'X', 'Y', 'Z'};

	ALERT(ret.size() >  size);
	srand(time(NULL));
	while (ret.size() < size) {
		int pos = random() % (sizeof(CH_POOL)/sizeof(CH_POOL[0]));

		ret = ret + CH_POOL[pos];
	}

	ret = ret;
	return ret;
}
