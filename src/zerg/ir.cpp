/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <stdlib.h>
#include "zerg.h"

IR::IR(std::string dst, ZergArgs *args) : Binary(dst, args->_pie_) {
	this->_param_nr_	= 0;
	this->_entry_		= args->_entry_;
	this->_symb_		= args->_symbol_;
}
IR::~IR(void) {
	if (!this->_only_ir_) {
		Binary::dump(this->_entry_, this->_symb_);
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
		IRToken *token = new IRToken(line);

		if (0 != token->length()) {
			this->emit(token);
		}

		delete token;
	}
}
void IR::emit(IRToken *token) {
	std::string op   = token->op(),
				dst  = token->dst(),
				src  = token->src(),
				idx  = token->idx(),
				size = token->size();

	return this->emit(op, dst, src, idx, size);
}
void IR::emit(std::string op, std::string _dst, std::string _src, std::string _idx, std::string _size) {
	std::string src = "", dst = "", idx = "";

	dst = this->regalloc(_dst, _size);
	src = __IR_DUMMY__ != _idx ? this->regalloc(_src) : this->regalloc(_src, _size);
	idx = this->regalloc(_idx);

	_D(LOG_INFO, "IR emit - %s %s %s %s %s",
			op.c_str(), dst.c_str(), src.c_str(), idx.c_str(), _size.c_str());

	if (op == "XCHG") {					/* (XCHG,  DST, SRC) */
		ALERT("" == dst || "" == src);
		_D(LOG_CRIT, "Not Implemented");
	} else if (op == "LOAD") {			/* (LOAD,  DST, SRC, IDX, SIZE) */
		/* Load data from memory with index if need */
		int pos = 0;
		char buff[BUFSIZ] = {0};

		if (src == _src) {						 /* save local variable */
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
			_D(LOG_BUG, "Need NOT assemble mov `%s` `%s`", dst.c_str(), src.c_str());
		}
	} else if (op == "STORE") {			/* (STORE, DST, SRC, IDX, SIZE) */
		/* Load data from memory with index if need */
		int pos = 0;
		char buff[BUFSIZ] = {0};

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
			ALERT(_stack_.size() == pos && ("" != idx && src != __IR_LOCAL_VAR__));
			if (_stack_.size() == pos) {
				/* save into _stack_ */
				_stack_.push_back(dst);
			}

			snprintf(buff, sizeof(buff), "[rbp-0X%X]", (pos+1) * 0x08);

			if (src == __IR_LOCAL_VAR__ && "" != idx) {
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
			_D(LOG_BUG, "Need NOT assemble mov `%s` `%s`", dst.c_str(), src.c_str());
		}
	} else if (op == "ADD") {			/* (ADD,   DST, SRC) */
		/* dst = dst + src */
		(*this) += new Instruction("add", dst, src);
	} else if (op == "SUB") {			/* (SUB,   DST, SRC) */
		/* dst = dst - src */
		(*this) += new Instruction("sub", dst, src);
	} else if (op == "MUL") {			/* (MUL,   DST, SRC) */
		/* dst = dst * src */
		(*this) += new Instruction("mul", dst, src);
	} else if (op == "DIV") {			/* (DIV,   DST, SRC) */
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
	} else if (op == "REM") {			/* (REM,   DST, SRC) */
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
	} else if (op == "INC") {			/* (ADD,   DST) */
		/* dst = dst + 1 */
		(*this) += new Instruction("inc", dst);
	} else if (op == "DEC") {			/* (DEC,   DST) */
		/* dst = dst - 1 */
		(*this) += new Instruction("dec", dst);
	} else if (op == "SHL") {			/* (SHL,   DST, SRC) */
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
	} else if (op == "SHR") {			/* (SHR,   DST, SRC) */
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
	} else if (op == "AND") {			/* (AND,   DST, SRC) */
		/* dst = dst & src */
		(*this) += new Instruction("and", dst, src);
	} else if (op == "OR") {			/* (OR,    DST, SRC) */
		/* dst = dst | src */
		(*this) += new Instruction("or", dst, src);
	} else if (op == "XOR") {			/* (XOR,   DST, SRC) */
		/* dst = dst ^ src */
		(*this) += new Instruction("xor", dst, src);
	} else if (op == "NOT") {			/* (NOT,   DST) */
		/* negative dst with 1's complement */
		(*this) += new Instruction("not", dst);
	} else if (op == "NEG") {			/* (NEG,   DST) */
		/* negative dst with 2's complement */
		(*this) += new Instruction("neg", dst);
	} else if (op == "ABS") {			/* (ABS,   DST) */
		/* dst = (dst XOR y) - y */
		std::string tmp = dst == "rsp" ? "rax" : "rsp";

		(*this) += new Instruction("xor", dst, tmp);
		(*this) += new Instruction("sub", dst, tmp);
	} else if (op == "EQ")  {			/* (EQ,    DST, SRC) */
		/* dst = dst eq src */
		std::string tmp;
		std::vector<std::string> regs = { REGISTERS };
		int pos = std::find(regs.begin(), regs.end(), dst) - regs.begin();

		ALERT(pos == regs.size());
		tmp = pos >= 32 ? regs[pos%8 + (8*8)] : regs[pos%8 + (8*3)];

		(*this) += new Instruction("cmp", dst, src);
		(*this) += new Instruction("setz", tmp);
		(*this) += new Instruction("and",  tmp, "0x1");
	} else if (op == "LS")  {			/* (EQ,    DST, SRC) */
		/* dst = dst < src */
		std::string tmp;
		std::vector<std::string> regs = { REGISTERS };
		int pos = std::find(regs.begin(), regs.end(), dst) - regs.begin();

		ALERT(pos == regs.size());
		tmp = pos >= 32 ? regs[pos%8 + (8*8)] : regs[pos%8 + (8*3)];

		(*this) += new Instruction("cmp", dst, src);
		(*this) += new Instruction("setl", tmp);
		(*this) += new Instruction("and",  tmp, "0x1");
	} else if (op == "LE")  {			/* (EQ,    DST, SRC) */
		/* dst = dst <= src */
		std::string tmp;
		std::vector<std::string> regs = { REGISTERS };
		int pos = std::find(regs.begin(), regs.end(), dst) - regs.begin();

		ALERT(pos == regs.size());
		tmp = pos >= 32 ? regs[pos%8 + (8*8)] : regs[pos%8 + (8*3)];

		(*this) += new Instruction("cmp", dst, src);
		(*this) += new Instruction("setle", tmp);
		(*this) += new Instruction("and",   tmp, "0x1");
	} else if (op == "GE")  {			/* (EQ,    DST, SRC) */
		/* dst = dst >= src */
		std::string tmp;
		std::vector<std::string> regs = { REGISTERS };
		int pos = std::find(regs.begin(), regs.end(), dst) - regs.begin();

		ALERT(pos == regs.size());
		tmp = pos >= 32 ? regs[pos%8 + (8*8)] : regs[pos%8 + (8*3)];

		(*this) += new Instruction("cmp", dst, src);
		(*this) += new Instruction("setge", tmp);
		(*this) += new Instruction("and",   tmp, "0x1");
	} else if (op == "GT")  {			/* (EQ,    DST, SRC) */
		/* dst = dst > src */
		std::string tmp;
		std::vector<std::string> regs = { REGISTERS };
		int pos = std::find(regs.begin(), regs.end(), dst) - regs.begin();

		ALERT(pos == regs.size());
		tmp = pos >= 32 ? regs[pos%8 + (8*8)] : regs[pos%8 + (8*3)];

		(*this) += new Instruction("cmp", dst, src);
		(*this) += new Instruction("setg", tmp);
		(*this) += new Instruction("and",  tmp, "0x1");
	} else if (op == "JMP") {			/* (JMP,   DST) */
		/* directly jump */
		(*this) += new Instruction("jmp", __IR_REFERENCE__ + dst);
	} else if (op == "JMP_TRUE") {		/* (JMP_TRUE, DST, SRC) */
		/* jump if true */
		ALERT("" == dst || "" == src);
		(*this) += new Instruction("cmp", src, "0x0");
		(*this) += new Instruction("jne", __IR_REFERENCE__ + dst);
	} else if (op == "JMP_FALSE") {		/* (JMP_FALSE, DST, SRC) */
		/* jump if false */
		ALERT("" == dst || "" == src);
		(*this) += new Instruction("cmp", src, "0x0");
		(*this) += new Instruction("je", __IR_REFERENCE__ + dst);
	} else if (op == "CALL") {			/* (CALL,  DST, SRC) */
		/* call produce */
		int nr = atoi(src.c_str());
		(*this) += new Instruction("call", __IR_REFERENCE__ + dst);

		if (0 != this->_param_nr_) {
			char buff[BUFSIZ] = {0};

			snprintf(buff, sizeof(buff), "0x%X", nr * 0x08);
			(*this) += new Instruction("add", "rsp", buff);
			this->_param_nr_ -= nr;
		}
	} else if (op == "RET") {			/* (RET,   DST) */
		/* 	return from procedure */
		if (""  != dst) {
			(*this) += new Instruction("mov", SYSCALL_REG, dst);
		}

		(*this) += new Instruction("ret");
	} else if (op == "PARAM") {			/* (PARAM, DST) */
		/* Save the parameter */
		int pos = 0;
		char buff[BUFSIZ] = {0};
		std::vector<std::string> regs = { "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9"};

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
	} else if (op == "LABEL") {			/* (LABEL, DST, SRC) */
		/* Set label or set variable */
		if ("" == src) {
			(*this) += new Instruction("asm", dst + ":");
		} else {
			/* NOTE - the string need to be a "STRING" */
			(*this) += new Instruction("asm", dst, "\"" + src + "\"");
		}
	} else if (op == "NOP") {			/* (NOP) */
		/* NOP */
		(*this) += new Instruction("nop");
	} else if (op == "INTERRUPT") {		/* (INTERRUPT) */
		/* Call system interrupt, and it is platform-dependent */
		std::vector<std::string> regs = { "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9"};

		while (0 != this->_param_nr_) {
			(*this) += new Instruction("pop", regs[--this->_param_nr_]);
		}
		(*this) += new Instruction("syscall");
		this->_param_nr_ = 0;
	} else if (op == "PROLOGUE") {		/* (PROLOGUE, NR) */
		this->_stack_.clear();
		this->resetreg();
		(*this) += new Instruction("push", "rbp");
		(*this) += new Instruction("mov", "rbp", "rsp");
		if ("" != dst) {
			/* save the number of local variable */
			(*this) += new Instruction("sub", "rsp", dst);
		}

		this->_param_nr_ = 0;
	} else if (op == "EPILOGUE") {		/* (EPILOGUE, NR) */
		if ("" != dst) {
			/* save the number of local variable */
			(*this) += new Instruction("add", "rsp", dst);
		}
		(*this) += new Instruction("pop", "rbp");
	} else if (op == "ASM") {
		(*this) += new Instruction(dst, src, idx);
	} else {
		_D(LOG_CRIT, "Not Implemented operators `%s`", op.c_str());
		return ;
	}

	this->regsave(src);
	this->regsave(idx);
}
std::string IR::randstr(int size, std::string prefix, std::string suffix) {
	/* generated a random label string */
	std::string ret = prefix;
	char CH_POOL[] = {	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
						'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
						'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
						'U', 'V', 'W', 'X', 'Y', 'Z'};

	ALERT(ret.size() >  size);
	srand(time(NULL));
	while (ret.size() + suffix.size() < size) {
		int pos = random() % (sizeof(CH_POOL)/sizeof(CH_POOL[0]));

		ret = ret + CH_POOL[pos];
	}

	ret = ret + suffix;
	return ret;
}
