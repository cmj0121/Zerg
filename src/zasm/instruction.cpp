/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include <sstream>
#include "zasm.h"

Instruction::Instruction(ZasmToken *cmd, ZasmToken *op1, ZasmToken *op2) {
	this->_inst_.push_back(cmd);
	if (NULL != op1) this->_inst_.push_back(op1);
	if (NULL != op2) this->_inst_.push_back(op2);

	this->_length_    = 0;
	this->assemble();
}
Instruction::Instruction(std::string cmd, std::string op1, std::string op2) {
	this->_inst_.push_back(new ZasmToken(cmd));
	if ("" != op1) this->_inst_.push_back(new ZasmToken(op1));
	if ("" != op2) this->_inst_.push_back(new ZasmToken(op2));

	this->_length_    = 0;
	this->assemble();
}

off_t Instruction::length(void) {
	/* Length of this instruction */
	if (this->cmd() == TOKEN_ASM && this->src()) {
		return this->src().unescape().size()+1;
	}
	return this->_length_;
}
std::string Instruction::label(void) {
	/* Reply label if exist */
	return this->_label_;
}
std::string Instruction::refer(void) {
	std::string ref;

	if (this->dst().isREF()) {
		ref = this->dst().raw();
	} else if (this->src().isREF()) {
		ref = this->src().raw();
	}

	return "" == ref ? "" : ref.substr(1, ref.size());
}
std::string Instruction::show(void) {
	char buff[BUFSIZ] = {0};
	const char *fmt[2] = {
		"\n%-7s  %-14s  %s",
		"%7s  %-14s  %s"};

	snprintf(buff, sizeof(buff),
		TOKEN_ASM == this->cmd().raw() ? fmt[0] : fmt[1],
		this->cmd().raw().c_str(),
		this->dst().raw().c_str(),
		this->src().raw().c_str());
	return std::string(buff);
}
bool Instruction::readdressable(void) {
	/* Reply this instruction is need to readdress or not */
	return this->cmd() != TOKEN_ASM && (this->dst().isREF() || this->src().isREF());
}
bool Instruction::isLabel(void) {
	return TOKEN_ASM == this->cmd().raw() && "" == this->src().raw();
}
Instruction& Instruction::operator << (std::fstream &dst) {
	if (this->cmd() == TOKEN_ASM && this->src()) {
		dst.write(this->src().unescape().c_str(), this->length());
	} else {
		dst.write((char *)this->_payload_, this->length());
	}

	std::stringstream ss;
	if (this->cmd() == TOKEN_ASM && this->src()) {
		ss << std::setw(7) << TOKEN_ASM << " " << this->dst().raw();
		_D(ZASM_LOG_DISASM, "%-32s - %s", ss.str().c_str(), this->src().unescape().c_str());
	} else if (this->cmd() == TOKEN_ASM) {
		_D(ZASM_LOG_DISASM, "%-7s %s", this->cmd().raw().c_str(), this->dst().raw().c_str());
	} else {
		std::stringstream src;
		for (int i=0; i < this->length(); ++i) {
			ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2)
				<< (int)this->_payload_[i] << " ";
		}

		src << std::setw(7) << this->cmd().raw()  <<  " "  <<  this->dst().raw()
				<<  " " << this->src().raw();
		_D(ZASM_LOG_DISASM, "%-32s - %s", src.str().c_str(), ss.str().c_str());
	}
	return (*this);
}

ZasmToken& Instruction::cmd(void) {
	/* This is MUST exists */
	return *this->_inst_[0];
}
ZasmToken& Instruction::dst(void) {
	/* Usually, this is exist and should be the destination */
	return (1 < this->_inst_.size()) ? *this->_inst_[1] : EMPTY_TOKEN;
}
ZasmToken& Instruction::src(void) {
	/* Usually, this is exist and should be the source */
	return (2 < this->_inst_.size()) ? *this->_inst_[2] : EMPTY_TOKEN;
}
off_t Instruction::offset(void) {
	/* Get the all offset in the current instruction */
	return this->dst().offset() ? this->dst().offset() : this->src().offset();
}
off_t Instruction::setIMM(std::string imm, int size, bool reset) {
	ZasmToken tmp(imm);
		_DEBUG();
	return this->setIMM(tmp.asInt(), size, reset);
}
off_t Instruction::setIMM(off_t imm, int size, bool reset) {
	off_t off = imm;

	_D(ZASM_LOG_DEBUG, "add immediate %llX, %d", imm, size);
	if (reset) {
		this->_length_ -= size;
	}

	while (size) {
		this->_payload_[this->_length_++] = (unsigned char)(off & 0xFF);
		size --;
		off >>= 8;
	}

	if (off != 0 && off != (off_t)-1) {
		_D(LOG_CRIT, "Not Implemented %llX %llX", imm, off);
		exit(-1);
	}

	return imm;
}

void Instruction::assemble(void) {
	int idx = 0;
	X86_64_INST inst;

	_D(ZASM_LOG_WARNING, "Assemble `%s` `%s` `%s`",
			this->cmd().raw().c_str(), this->dst().raw().c_str(), this->src().raw().c_str());
	if (this->cmd() == TOKEN_ASM) {
		if (this->src()) {
			this->_label_ = this->dst().raw();
		} else {
			this->_label_ = this->dst().raw().substr(0, this->dst().raw().size()-1);
		}
	} else {
		for (idx = 0; idx < ARRAY_SIZE(InstructionSets); ++idx) {
			inst = InstructionSets[idx];

			if (this->cmd() != inst.cmd) {
				continue;
			} else if (! this->dst().match(inst.op1) || ! this->src().match(inst.op2)) {
				_D(ZASM_LOG_INFO, "%s 0x%02X not match %d %d",
								inst.cmd, inst.opcode,
								this->dst().match(inst.op1),
								this->src().match(inst.op2));
				continue;
			}
			_D(ZASM_LOG_INFO, "%s 0x%02X match %d %d",
							inst.cmd, inst.opcode,
							this->dst().match(inst.op1),
							this->src().match(inst.op2));

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
			this->cmd().raw().c_str(), this->dst().raw().c_str(), this->src().raw().c_str());
		exit(-1);
	}
}
