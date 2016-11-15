/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include "zerg.h"
IRToken::IRToken() {
}
IRToken::IRToken(std::string op, std::string dst, std::string src, std::string extra) {
	this->_op_  = op;
	this->_dst_ = dst;
	this->_src_ = src;
	this->_ext_ = extra;
}
std::fstream& operator<< (std::fstream &stream, const IRToken &src) {
	stream << "(" << src._op_;
	if ("" != src._dst_) stream << ", " << src._dst_;
	if ("" != src._src_) stream << ", " << src._src_;
	if ("" != src._ext_) stream << ", " << src._ext_;
	stream << ")";

	return stream;
}

IR::IR(std::string dst, off_t entry) : Binary(dst), _param_nr_(0), _entry_(entry) {
}
IR::~IR(void) {
	if (!this->_only_ir_) {
		Binary::dump(this->_entry_);
	}
}

void IR::emit(IRToken token) {
	/* wrapper for the emmitter using IR token */
	std::string dst = token.dst(), src = token.src(), extra = token.extra();

	return this->emit(token.op(), dst, src, extra);
}
void IR::emit(std::string op, std::string &dst, std::string &src, std::string &extra) {
	static std::vector<std::string> stack;

	dst   = this->regalloc(dst);
	src   = this->regalloc(src);
	extra = this->regalloc(extra);

	_D(LOG_INFO, "IR emit - %s %s %s %s",
			op.c_str(), dst.c_str(), src.c_str(), extra.c_str());

	if (op == "COPY") {					/* (COPY,  DST, SRC) */
		/* Copy data from src to dst */
		if ("rsi" == dst && '&' == src[0]) {
			(*this) += new Instruction("lea", dst, src);
		} else {
			(*this) += new Instruction("mov", dst, src);
		}
	} else if (op == "LOAD") {			/* (LOAD,  DST, SRC, EXTRA) */
		/* Load data from memory with index if need */
		int pos = 0;
		char buff[BUFSIZ] = {0};

		if (__IR_LOCAL_VAR__ == extra) {
			/* save local variable */
			pos = std::find(stack.begin(), stack.end(), src) - stack.begin();

			if (stack.size() == pos) {
				/* save into stack */
				_D(LOG_CRIT, "Local VAR `%s` not declare", src.c_str());
			} else {
				snprintf(buff, sizeof(buff), "[rbp-0X%X]", pos * 0x08);
				(*this) += new Instruction("mov", dst, buff);
			}
		} else if (__IR_GLOBAL_VAR__ == extra) {
			/* save global variable */
			_D(LOG_CRIT, "Not Implemented");
		} else if (dst != src) {
			(*this) += new Instruction("mov", dst, src);
		}
	} else if (op == "STORE") {			/* (STORE, DST, SRC, EXTRA) */
		/* Load data from memory with index if need */
		int pos = 0;
		char buff[BUFSIZ] = {0};

		if (__IR_LOCAL_VAR__ == extra) {
			/* save local variable */
			pos = std::find(stack.begin(), stack.end(), dst) - stack.begin();

			if (stack.size() == pos) {
				/* save into stack */
				stack.push_back(dst);
			}
			snprintf(buff, sizeof(buff), "[rbp-0X%X]", pos * 0x08);
			(*this) += new Instruction("mov", buff, src);
		} else if (__IR_GLOBAL_VAR__ == extra) {
			/* save global variable */
			_D(LOG_CRIT, "Not Implemented");
		} else if (dst != src) {
			(*this) += new Instruction("mov", dst, src);
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
		(*this) += new Instruction("jmp", dst);
	} else if (op == "CALL") {			/* (CALL,  DST) */
		/* call produce */
		(*this) += new Instruction("call", dst);
	} else if (op == "RET") {			/* (RET) */
		/* 	return from procedure */
		(*this) += new Instruction("ret");
	} else if (op == "PARAM") {			/* (PARAM, DST) */
		/* Save the parameter */
		std::vector<std::string> regs = { "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9"};

		if ("rsi" == regs[this->_param_nr_] && '&' == dst[0]) {
			ALERT(this->_param_nr_ == regs.size());

			(*this) += new Instruction("lea", regs[this->_param_nr_], dst);
			(*this) += new Instruction("push", regs[this->_param_nr_]);
		} else {
			(*this) += new Instruction("push", dst);
		}
		this->_param_nr_ ++;
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
	} else if (op == "PROLOGUE") {		/* (PROLOGUE, NR) */
		(*this) += new Instruction("push", "rbp");
		(*this) += new Instruction("mov", "rbp", "rsp");
		if ("" != dst) {
			/* save the number of local variable */
			(*this) += new Instruction("sub", "rsp", dst);
		}
	} else if (op == "EPILOGUE") {		/* (EPILOGUE, NR) */
		if ("" != dst) {
			/* save the number of local variable */
			(*this) += new Instruction("add", "rsp", dst);
		}
		(*this) += new Instruction("pop", "rbp");

		/* FIXME */
		(*this) += new Instruction("mov", "rax", "0x2000001");
		(*this) += new Instruction("syscall");

		(*this) += new Instruction("ret");
	} else if (op == "ASM") {
		(*this) += new Instruction(dst, src, extra);
	} else {
		_D(LOG_CRIT, "Not Implemented operators `%s`", op.c_str());
		exit(-1);
	}
}
