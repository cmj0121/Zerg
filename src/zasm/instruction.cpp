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
	this->assemble();
}

off_t Instruction::length(void) {
	/* Length of this instruction */
	if (this->cmd == TOKEN_ASM && this->src) {
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

	return "" == ref ? "" : ref.substr(1, ref.size());
}
bool Instruction::readdressable(void) {
	/* Reply this instruction is need to readdress or not */
	return this->cmd != TOKEN_ASM && (this->dst.isREF() || this->src.isREF());
}
bool Instruction::isShowLabel(void) {
	std::string symb = this->label();

	return (TOKEN_ASM == this->cmd.raw() && "" == this->src.raw()) &&
		(ZASM_ENTRY_POINT == symb || ("" != symb && '.' != symb[0] && '_' != symb[0]));
}
Instruction& Instruction::operator << (std::fstream &dst) {
	if (this->cmd == TOKEN_ASM && this->src) {
		dst.write(this->src.unescape().c_str(), this->length());
	} else {
		dst.write((char *)this->_payload_, this->length());
	}

	std::stringstream ss;
	if (this->cmd == TOKEN_ASM && this->src) {
		ss << std::setw(7) << TOKEN_ASM << " " << this->dst.raw();
		_D(LOG_DISASM, "%-32s - %s", ss.str().c_str(), this->src.unescape().c_str());
	} else if (this->cmd == TOKEN_ASM) {
		_D(LOG_DISASM, "%-7s %s", this->cmd.raw().c_str(), this->dst.raw().c_str());
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

	_D(LOG_DEBUG, "add immediate " OFF_T ", %d", imm, size);
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

	_D(LOG_INFO, "Assemble `%s` `%s` `%s`",
											this->cmd.raw().c_str(),
											this->dst.raw().c_str(),
											this->src.raw().c_str());
	if (this->cmd == TOKEN_ASM) {
		if (this->src) {
			this->_label_ = this->dst.raw();
		} else {
			this->_label_ = this->dst.raw().substr(0, this->dst.raw().size()-1);
		}
	} else {
		for (idx = 0; idx < ARRAY_SIZE(InstructionSets); ++idx) {
			inst = InstructionSets[idx];

			if (this->cmd != inst.cmd) {
				continue;
			} else if (! this->dst.match(inst.op1) || ! this->src.match(inst.op2)) {
				_D(LOG_INFO, "%s 0x%02X not match %d %d",
								inst.cmd, inst.opcode,
								this->dst.match(inst.op1),
								this->src.match(inst.op2));
				continue;
			}
			_D(LOG_INFO, "%s 0x%02X match %d %d",
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
	}

	if (idx == ARRAY_SIZE(InstructionSets)) {
		_D(LOG_CRIT, "Not Implemented `%s` `%s` `%s`",
			this->cmd.raw().c_str(), this->dst.raw().c_str(), this->src.raw().c_str());
		exit(-1);
	}
}
