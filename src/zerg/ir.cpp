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
	return this->emit(token.op(), token.dst(), token.src(), token.extra());
}
void IR::emit(std::string op, std::string dst, std::string src, std::string extra) {
	static std::vector<std::string> stack;

	dst = this->regalloc(dst);
	src = this->regalloc(src);

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
		} else {
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
				(*this) += new Instruction("push", src);
			} else {
				snprintf(buff, sizeof(buff), "[rbp-0X%X]", pos * 0x08);
				(*this) += new Instruction("mov", src, buff);
			}
		} else if (__IR_GLOBAL_VAR__ == extra) {
			/* save global variable */
			_D(LOG_CRIT, "Not Implemented");
		} else {
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
		(*this) += new Instruction("mov", "rax", dst);
		(*this) += new Instruction("xor", "rdx", "rdx");
		(*this) += new Instruction("div", src);
		(*this) += new Instruction("mov", dst, "rax");
	} else if (op == "REM") {			/* (REM,   DST, SRC) */
		/* dst = dst / src */
		(*this) += new Instruction("mov", "rax", dst);
		(*this) += new Instruction("xor", "rdx", "rdx");
		(*this) += new Instruction("div", src);
		(*this) += new Instruction("mov", dst, "rdx");
	} else if (op == "INC") {			/* (ADD,   DST) */
		/* dst = dst + 1 */
		(*this) += new Instruction("inc", dst);
	} else if (op == "DEC") {			/* (DEC,   DST) */
		/* dst = dst - 1 */
		(*this) += new Instruction("dec", dst);
	} else if (op == "SHL") {			/* (SHL,   DST, SRC) */
		/* dst = dst << src */
		(*this) += new Instruction("shl", dst, src);
	} else if (op == "SHR") {			/* (SHR,   DST, SRC) */
		/* dst = dst >> src */
		(*this) += new Instruction("shr", dst, src);
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
	} else if (op == "CMP") {			/* (CMP,   VAR, VAR) */
		/* raw compare */
		(*this) += new Instruction("cmp", dst, src);
	} else if (op == "JMP") {			/* (JMP,   DST) */
		/* directly jump */
		(*this) += new Instruction("jmp", dst);
	} else if (op == "JEQ") {			/* (JEQ,   DST) */
		/* directly jump */
		(*this) += new Instruction("je", dst);
	} else if (op == "JNEQ") {			/* (JNEQ,  DST) */
		/* directly jump */
		(*this) += new Instruction("jne", dst);
	} else if (op == "JLS") {			/* (JLS,   DST) */
		/* directly jump */
		(*this) += new Instruction("jl", dst);
	} else if (op == "JLSE") {			/* (JLSE,  DST) */
		/* directly jump */
		(*this) += new Instruction("jle", dst);
	} else if (op == "JGT") {			/* (JGT,   DST) */
		/* directly jump */
		(*this) += new Instruction("jg", dst);
	} else if (op == "JGTE") {			/* (JGTE,  DST) */
		/* jump if greater or equal */
		(*this) += new Instruction("jge", dst);
	} else if (op == "CALL") {			/* (CALL,  DST) */
		/* call produce */
		(*this) += new Instruction("call", dst);
	} else if (op == "RET") {			/* (RET) */
		/* 	return from procedure */
		(*this) += new Instruction("ret");
	} else if (op == "PARAM") {			/* (PARAM, DST) */
		/* Save the parameter */
		std::vector<std::string> regs = { "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9"};

		this->emit("COPY", regs[this->_param_nr_++], dst);
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
		(*this) += new Instruction("syscall");
		this->_param_nr_ = 0;
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
	} else {
		_D(LOG_CRIT, "Not Implemented operators `%s`", op.c_str());
		exit(-1);
	}
}
