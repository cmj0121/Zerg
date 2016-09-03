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

IR::IR(std::string dst) : Binary(dst), _param_nr_(0) {
	/* Mock Testing */
	IR::emit(IRToken("PARAM", "0x2000004"));
	IR::emit(IRToken("PARAM", "0x01"));
	IR::emit(IRToken("PARAM", "&str"));
	IR::emit(IRToken("PARAM", "0x0C"));
	IR::emit(IRToken("INTERRUPT"));

	IR::emit("PARAM", "0x2000001");
	IR::emit("PARAM", "0x04");
	IR::emit("INTERRUPT");

	IR::emit("LABEL", "str", "ZasmP IR\\n");
	Binary::dump();
}

void IR::emit(IRToken token) {
	/* wrapper for the emmitter using IR token */
	return this->emit(token.op(), token.dst(), token.src(), token.extra());
}
void IR::emit(std::string op, std::string dst, std::string src, std::string extra) {
	if (op == "COPY") {					/* (COPY,  DST, SRC) */
		/* Copy data from src to dst */
		if ("rsi" == dst && '&' == src[0]) {
			(*this) += new Instruction("lea", dst, src);
		} else {
			(*this) += new Instruction("mov", dst, src);
		}
	} else if (op == "LOAD") {			/* (LOAD,  DST, SRC, EXTRA) */
		/* Load data from memory with index if need */
		char buff[BUFSIZ] = {0};

		snprintf(buff, sizeof(buff), "[%s%s%s]", src.c_str(), extra == "" ? "" : "+", extra.c_str());

		(*this) += new Instruction("mov", dst, buff);
	} else if (op == "STORE") {			/* (STORE, DST, SRC, EXTRA) */
		/* Load data from memory with index if need */
		char buff[BUFSIZ] = {0};

		snprintf(buff, sizeof(buff), "[%s%s%s]", dst.c_str(), extra == "" ? "" : "+", extra.c_str());

		(*this) += new Instruction("mov", buff, src);
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
		(*this) += new Instruction("xor", "rdx");
		(*this) += new Instruction("div", src);
		(*this) += new Instruction("mov", dst, "rax");
	} else if (op == "REM") {			/* (REM,   DST, SRC) */
		/* dst = dst / src */
		(*this) += new Instruction("mov", "rax", dst);
		(*this) += new Instruction("xor", "rdx");
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
	} else if (op == "PARAM") {			/* (PARAM, DST) */
		/* Save the parameter */
		this->_param_[this->_param_nr_ ++] = dst;
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

		for (int i = 0; i < this->_param_nr_; ++i) {
			this->emit("COPY", regs[i], this->_param_[i]);
		}
		(*this) += new Instruction("syscall");
		this->_param_nr_ = 0;
	} else {
		_D(LOG_CRIT, "Not Implemented operators `%s`", op.c_str());
		exit(-1);
	}
}
