/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include "zerg.h"

Zerg::Zerg(std::string src, std::string dst) : Binary(dst) {
	(*this) += new Instruction("mov", "rax", "0x2000001");
	(*this) += new Instruction("mov", "rdi", "0x04");
	(*this) += new Instruction("syscall");
	Binary::dump();
}
