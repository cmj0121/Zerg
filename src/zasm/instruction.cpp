/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include <sstream>
#include "zasm.h"

static X86_64_INST InstructionSets[] = {
	#define _ASM_OP(cmd, _x, _y, _z, _u) 		{ #cmd, _x, _y, _z, _u },
	#define _ASM_OP_PF_TB(_x, _y, _z, _u, _w)	_ASM_OP(_x, _y, _z, _u, _w | INST_PF_TWOBYTE)
	#define _ASM_OP2(cmd, op, x, y)				_ASM_OP(cmd, op, x, y, INST_NONE)
	#define _ASM_OP1(cmd, op, x)				_ASM_OP2(cmd, op, x, INST_NONE, INST_NONE)
	#define _ASM_OP0(cmd, op) 					_ASM_OP1(cmd, op, INST_NONE, INST_NONE, INST_NONE)

	//#  include "zasm/x86_64_opcodes.h"

	#undef _ASM_OP0
	#undef _ASM_OP1
	#undef _ASM_OP2
	#undef _ASM_OP_PF_TB
	#undef _ASM_OP
};


Instruction::Instruction(STRING cmd, STRING op1, STRING op2, int mode) {
	this->cmd = InstToken(cmd);
	this->dst = InstToken(op1);
	this->src = InstToken(op2);
	this->_length_    = 0;
	this->_repeat_    = 0;
	this->_absaddr_   = false;

	#ifdef __x86_64__
	mode = 0 == mode ? X86_PROTECTED_MODE : mode;
	#endif /* __x86_64__ */

	if (':' == cmd[cmd.size()-1]) {
		/* NOTE - cannot directly modified the string */

		this->_label_ = cmd.substr(0, cmd.size()-1);
	} else if (ZASM_DEFINE == cmd) {
		this->_label_ = op1;
	} else if (ZASM_REPEAT == cmd) {
		if (!this->src.isIMM())			_D(LOG_CRIT, "Not Implemented");
		if (!this->src.isIMMRange())	this->setRepeat(this->src.asInt());
	} else {
		/* assemble the machine code by each platform */
		this->assemble(mode);
	}
}

off_t Instruction::length(void) {
	/* Length of this instruction */
	if ("" != this->_label_ && "" != this->src.raw()) {
		return this->src.unescape().size()+1;
	} else if (0 != this->_repeat_) {
		if (this->dst.isIMM()) return this->_repeat_;
		return this->_repeat_ * this->dst.unescape().size();
	}
	return this->_length_;
}
std::string Instruction::label(void) {
	/* Reply label if exist */
	return this->_label_;
}
std::string Instruction::refer(void) {
	std::string ref;

	ref = this->dst.isREF() ? this->dst.raw() : this->src.raw();
	return 0 < ref.size() && ZASM_REFERENCE == ref[0] ? ref.substr(1) : ref;
}
bool Instruction::readdressable(void) {
	/* Reply this instruction is need to readdress or not */
	return this->dst.isREF() || this->src.isREF() || this->src.isIMMRange();
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
off_t Instruction::setRepeat(off_t imm) {
	this->_repeat_ = imm;
	return this->_repeat_;
}

void Instruction::assemble(int mode) {
	unsigned int idx = 0;
	X86_64_INST inst;

	for (idx = 0; idx < ARRAY_SIZE(InstructionSets); ++idx) {
		inst = InstructionSets[idx];

		if (this->cmd != inst.cmd) continue;

		goto END;
	}

	_D(LOG_CRIT, "Not Implemented `%s` `%s` `%s`",
		this->cmd.raw().c_str(), this->dst.raw().c_str(), this->src.raw().c_str());
	exit(-1);
END:
	return;
}
Instruction& operator << (std::fstream &dst, const Instruction &inst) {
	if ("" != this->_label_ && "" != this->src.raw()) {
		dst.write(this->src.unescape().c_str(), this->length());
	} else if (0 != this->_repeat_) {
		ALERT(0 > this->_repeat_);

		for (off_t i = 0; i < this->_repeat_; ++i) {
			if (this->dst.isIMM()) {
				this->_payload_[0] = this->dst.isIMM();
				dst.write((char *)this->_payload_, 1);
			} else {
				dst.write(this->dst.unescape().c_str(), this->dst.unescape().size());
			}
		}
	} else {
		dst.write((char *)this->_payload_, this->length());
	}

	std::stringstream ss;
	if ("" != this->_label_ && "" != this->src.raw()) {
		ss << std::setw(7) << std::left << ZASM_DEFINE << " " << this->dst.raw();
		_D(LOG_DISASM, "%-32s - %s", ss.str().c_str(), this->src.raw().c_str());
	} else if ("" != this->_label_) {
		_D(LOG_DISASM, "%s:", this->_label_.c_str());
	} else if (this->_repeat_) {
		std::stringstream src;
		src << std::setw(7) << this->cmd.raw()  <<  " "  <<  this->dst.raw()
				<<  " " << this->src.raw();
		if (this->dst.isIMM()) {
			_D(LOG_DISASM, "%-32s - %02X ... (" OFF_T ")",
										src.str().c_str(), _payload_[0], this->_repeat_);
		} else {
			_D(LOG_DISASM, "%-32s - %s ... (" OFF_T ")",
							src.str().c_str(), this->dst.raw().c_str(), this->_repeat_);
		}
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
