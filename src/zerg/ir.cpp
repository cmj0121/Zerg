/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include "zerg.h"

Tuple::Tuple(std::string op, std::string dst, std::string x, std::string y) {
	this->_op_  = op;
	this->_dst_ = dst;
	this->_x_   = x;
	this->_y_   = y;
}
std::fstream& operator << (std::fstream &stream, const Tuple &src) {
	stream << "(" << src._op_ ;
	if ("" != src._dst_) stream << ", " << src._dst_;
	if ("" != src._x_)   stream << ", " << src._x_;
	if ("" != src._y_)   stream << ", " << src._y_;
	stream << ")";

	return stream;
}

IR::IR(std::string dst) : Binary(dst) {
	this->_operators_ = {
		/* value copy */
		"COPY",		/* (COPY,  DST, SRC) */
		"LOAD",		/* (LOAD,  DST, SRC) or (LOAD,  DST, SRC, INDEX) */
		"STORE",	/* (STORE, DST, SRC) or (STORE, DST, SRC, INDEX) */

		/* arithmetic */
		"ADD",			/* (ADD, DST, SRC) */
		"SUB",			/* (SUB, DST, SRC) */
		"MUL",			/* (MUL, DST, SRC) */
		"DIV",			/* (DIV, DST, SRC) */
		"REM",			/* (REM, DST, SRC) */
		"INC",			/* (INC, DST) */
		"DEC",			/* (DEC, DST) */

		/* bitwise operator */
		"SHL",			/* (SHL, DST, NR) */
		"SHR",			/* (SHR, DST, NR) */
		"AND",			/* (AND, DST, SRC) */
		"OR",			/* (OR,  DST, SRC) */
		"XOR",			/* (XOR, DST, SRC) */
		"NOT",			/* (NOT, DST) */
		"NEG",			/* (NEG, DST) */
		"ABS",			/* (ABS, DST) */

		/* logical compare */
		"CMP",			/* (CMP, SRC 1, SRC 2) */
		"EQ",			/* (EQ,  SRC 1, SRC 2) */
		"NEG",			/* (NEG, SRC 1, SRC 2) */
		"LS",			/* (LS,  SRC 1, SRC 2) */
		"LSE",			/* (LSE, SRC 1, SRC 2) */
		"GT",			/* (GT,  SRC 1, SRC 2) */
		"GTE",			/* (GTE, SRC 1, SRC 2) */

		/* condition control */
		"JMP",			/* (JMP,  DST) */
		"JEQ",			/* (JEQ,  DST) */
		"JNEQ",			/* (JNEQ, DST) */
		"JLS",			/* (JLS,  DST) */
		"JLSE",			/* (JLSE, DST) */
		"JGT",  		/* (JGT,  DST) */
		"JGTE",			/* (JGTE, DST) */

		/* subroutine */
		"CALL",			/* (CALL, DST) */
		"RET",			/* (RET) */
		"PARAM",		/* (PARAM, VAR, NR) */

		/* int - float transfer */
		"ITOF",			/* (ITOF, DST, SRC) */
		"FTOI",			/* (FTOI, DST, SRC) */

		/* extra pseudo operators */
		"LABEL",		/* (LABEL, DST) */
		"NOP",			/* (NOP) */
		"INTERRUPT",	/* (INTERRUPT) */
	};

	(*this) += new Instruction("mov", "rax", "0x2000001");
	(*this) += new Instruction("mov", "rdi", "0x04");
	(*this) += new Instruction("syscall");
	Binary::dump();
}
