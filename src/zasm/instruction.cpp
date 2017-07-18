/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include <sstream>
#include "zasm.h"

static X86_64_INST InstructionSets[] = {
	#define INST_ASM_OP(cmd, op, x, y, flag) 			{ #cmd, op, x, y, flag },
	#define INST_ASM_OP_TWOBYTE(cmd, op, x, y, flag)	INST_ASM_OP( cmd, op, x, y, flag | INST_TWO_BYTE)
	#define INST_ASM_OP0(cmd, op) 						INST_ASM_OP( cmd, op, INST_NONE, INST_NONE, INST_NONE)
	#define INST_ASM_OP1(cmd, op, x)					INST_ASM_OP( cmd, op, x,         INST_NONE, INST_NONE)
	#define INST_ASM_OP2(cmd, op, x, y)					INST_ASM_OP( cmd, op, x,         y,		    INST_NONE)

	#  include "zasm/x86_64_opcodes.h"

	#undef INST_ASM_OP
	#undef INST_ASM_OP_TWOBYTE
	#undef INST_ASM_OP0
	#undef INST_ASM_OP1
};


Instruction::Instruction(std::string cmd, std::string op1, std::string op2) {
	this->cmd = InstToken(cmd);
	this->dst = InstToken(op1);
	this->src = InstToken(op2);
	this->_length_    = 0;

	if (':' == cmd[cmd.size()-1] && "" == op1 && "" == op2) {
		/* NOTE - cannot directly modified the string */
		this->_label_ = cmd.substr(0, cmd.size()-1);
	} else if (ZASM_DEFINE == cmd) {
		this->_label_ = op1;
	} else {
		/* assemble the machine code by each platform */
		this->assemble();
	}
}

off_t Instruction::length(void) {
	/* Length of this instruction */
	if ("" != this->_label_ && "" != this->src.raw()) {
		return this->src.unescape().size()+1;
	}
	return this->_length_;
}
std::string Instruction::label(void) {
	/* Reply label if exist */
	return this->_label_;
}
std::string Instruction::refer(void) {
	std::string ref;

	if (this->dst.isREF()) {
		ref = this->dst.raw();
	} else if (this->src.isREF()) {
		ref = this->src.raw();
	}

	return "" == ref ? "" : ref.substr(1);
}
bool Instruction::readdressable(void) {
	/* Reply this instruction is need to readdress or not */
	return this->dst.isREF() || this->src.isREF();
}
bool Instruction::isShowLabel(void) {
	std::string symb = this->label();

	return  "" == this->src.raw() &&
		(ZASM_ENTRY_POINT == symb || ZASM_MAIN_FUNCTION == symb ||
		("" != symb && '.' != symb[0] && '_' != symb[0]));
}
Instruction& Instruction::operator << (std::fstream &dst) {
	if ("" != this->_label_ && "" != this->src.raw()) {
		dst.write(this->src.unescape().c_str(), this->length());
	} else {
		dst.write((char *)this->_payload_, this->length());
	}

	std::stringstream ss;
	if ("" != this->_label_ && "" != this->src.raw()) {
		ss << std::setw(7) << std::left << ZASM_DEFINE << " " << this->dst.raw();
		_D(LOG_DISASM, "%-32s - %s", ss.str().c_str(), this->src.unescape().c_str());
	} else if ("" != this->_label_) {
		_D(LOG_DISASM, "%s:", this->_label_.c_str());
	} else {
		std::stringstream src;
		for (int i=0; i < this->length(); ++i) {
			ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2)
				<< (int)this->_payload_[i] << " ";
		}

		src << std::setw(7) << this->cmd.raw()  <<  " "  <<  this->dst.raw()
				<<  " " << this->src.raw();
		_D(LOG_DISASM, "%-32s - %s", src.str().c_str(), ss.str().c_str());
	}
	return (*this);
}

off_t Instruction::offset(void) {
	/* Get the all offset in the current instruction */
	return this->dst.offset() ? this->dst.offset() : this->src.offset();
}
off_t Instruction::setIMM(off_t imm, int size, bool reset) {
	off_t off = imm;

	_D(LOG_ZASM_DEBUG, "add immediate " OFF_T ", %d", imm, size);
	if (reset) {
		this->_length_ -= size;
	}

	while (size) {
		this->_payload_[this->_length_++] = (unsigned char)(off & 0xFF);
		size --;
		off >>= 8;
	}

	if (off != 0 && off != (off_t)-1) {
		_D(LOG_CRIT, "Not Implemented " OFF_T " " OFF_T, imm, off);
		exit(-1);
	}

	return imm;
}

void Instruction::assemble(void) {
	unsigned int idx = 0;
	X86_64_INST inst;

	for (idx = 0; idx < ARRAY_SIZE(InstructionSets); ++idx) {
		inst = InstructionSets[idx];

		if (this->cmd != inst.cmd) {
			continue;
		} else if (! this->dst.match(inst.op1) || ! this->src.match(inst.op2)) {
			_D(LOG_ZASM_INFO, "%s 0x%02X not match %d %d",
							inst.cmd, inst.opcode,
							this->dst.match(inst.op1),
							this->src.match(inst.op2));
			continue;
		}
		_D(LOG_ZASM_INFO, "%s 0x%02X match %d %d",
						inst.cmd, inst.opcode,
						this->dst.match(inst.op1),
						this->src.match(inst.op2));

		#ifdef __x86_64__
			legacyPrefix(inst);
			opcode(inst);
			modRW(inst);
			displacement(inst);
			immediate(inst);
		#endif /* __x86_64__ */
		break;
	}

	if (idx == ARRAY_SIZE(InstructionSets)) {
		_D(LOG_CRIT, "Not Implemented `%s` `%s` `%s`",
			this->cmd.raw().c_str(), this->dst.raw().c_str(), this->src.raw().c_str());
		exit(-1);
	}
}
