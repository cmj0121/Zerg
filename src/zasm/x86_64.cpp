#ifdef __x86_64__
/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include <sstream>
#include "zasm.h"

#include "zasm/x86_64_inst.h"

/* X86-64 Instruction Encoding - Max to 15 bits
 *
 * 0               4        6     7     8                         16
 * |---------------|--------|-----|-----|--------------------------|
 *   Legacy Prefix   OpCode   Mod   SIB   Displacement / Immediate
 */

void Instruction::legacyPrefix(X86_64_INST &inst) {
	/* NOTE - Legacy Prefix (0~4) */
	if (4 == this->dst().size() || 4 == this->src().size()) {
		int REX_B = 0, REX_X = 0, REX_R = 0, REX_W = 0;

		/*
			7             4               0
			+-------------+---+---+---+---+
			| 0  1  0  0  | W | R | X | B |
			+-------------+---+---+---+---+

			REX.B	Extension to the Mod R/W.rm field or the SIB.base field
			REX.X	Extension to the SIB.index field
			REX.R	Extension to the Mod R/W.reg field
			REX.W	64-bit operand size is used.
		 */
		if (this->dst().isREG() && (this->src().isREG() || this->src().isIMM())) {
			if (this->dst().isEXT())
				REX_B = 1;
		} else if (this->dst().isREG() && "" == this->src().raw()) {
			if (this->dst().isEXT())
				REX_B = 1;
		} else if (this->dst().isMEM() && this->dst().isEXT()) {
			REX_B = 1;
		} else if (this->src().isMEM() && this->dst().isREG() &&this->src().isEXT()) {
			REX_B = 1;
		}

		if (this->src().isREG() && this->dst().isREG()) {
			if (this->src().isEXT())
				REX_R = 1;
		} else if (this->src().isMEM() && ! this->src().isREF() && this->dst().isEXT()) {
			REX_R = 1;
		} else if (this->dst().isMEM() && this->src().isREG() && this->src().isEXT()) {
			REX_R = 1;
		}

		if (0 == (inst.flags & INST_SECONDARY)) {
			REX_W = 1;
		}


		if (REX_B || REX_X || REX_R || REX_W) {
			_payload_[_length_++] = 0x40 | (REX_W << 3) | (REX_R << 2) | (REX_X << 1) | REX_B;
			_D(ZASM_LOG_WARNING, "Legacy Prefix - %02X", _payload_[_length_-1]);
		}
	}
}
void Instruction::opcode(X86_64_INST &inst) {
	/* NOTE  - Opcode        (1~2) */
	if (INST_TWO_BYTE & inst.flags) {
		_D(ZASM_LOG_WARNING, "Two Byte      - 0F");
		_payload_[_length_++] = 0x0F;
	}
	_payload_[_length_++] = inst.opcode;

	if (inst.flags & INST_SECONDARY && this->dst()) {
		_payload_[_length_-1] |= this->dst().asInt();
	}
	_D(ZASM_LOG_WARNING, "Opcode        - %02X", inst.opcode);

	/* NOTE - Secondary Opcode  or Opcode Extension */
	if (inst.flags & INST_OPCODE_EXT) {
		_payload_[_length_++] = (this->dst().isREG() ? 0x03 : 0x0) << 6;

		if (this->dst().isMEM() && this->dst().isMEM2()) {
			_payload_[_length_-1] |= 0x14;
		}
		if (this->dst().isMEM() && this->dst().offset()) {
			_payload_[_length_-1] |= 0x40;
		}

		switch(this->dst().asInt()) {
			case 4: case 5:
				_payload_[_length_-1] |= 0x40;
			default:
				if (0x54 != _payload_[_length_-1])
				_payload_[_length_-1] |= this->dst().asInt();
				break;
		}
		if (INST_OPCODE_MULTI & inst.flags) {
			_payload_[_length_-1] |= this->dst().asInt()  << 3;	// Extension
		} else {
			_payload_[_length_-1] |= (inst.flags & 0xF) << 3;	// Extension
		}
		_D(ZASM_LOG_WARNING, "Opcode Ext    - %02X", _payload_[_length_-1]);
	}
}
void Instruction::modRW(X86_64_INST &inst) {
	/* 7     5         2        0
	 * |-----|---------|--------|
	 * | mod |   reg   |   rm   |
	 * |-----|---------|--------|
	 *
	 * mod -
	 *        Usually is b11 which register direct addressing, otherwise
	 *        register-indirect addressing mode used.
	 * reg -
	 *        1) 3bit opcode extension
	 *        2) 3bit register reference
	 * rm  -
	 *        Direct or indirect register operand, optionally with a displacement
	 */
	int mod = 0x0, reg = 0x0, rm = 0x0;

	if (! this->dst() || ! this->src()) {
		return ;
	#ifdef __x86_64__
	} else if (INST_REG_SPECIFY & inst.op2) {
		return ;
	#endif /* __x86_64__ */
	} else if (this->dst().isREG() && this->src().isREG()) {
		mod = 0x03;
		reg = this->src().asInt();
		rm  = this->dst().asInt();

		if (inst.flags & INST_REG_SWAP) {
			reg ^= rm;
			rm  ^= reg;
			reg ^= rm;
		}

		_payload_[_length_++] = (mod & 0x3) << 6 | (reg & 0x7) << 3 | (rm & 0x7);
		_D(ZASM_LOG_WARNING, "Mod R/W       - %02X", _payload_[_length_-1]);
	} else if (this->dst().isMEM2() || this->src().isMEM2()) {
		ZasmToken *token = this->dst().isMEM2() ? this->dst().indexReg() : this->src().indexReg();

		if (this->offset()) {
			mod = 0 == (~0x7F & this->offset()) ? 0x01 : 0x02;
		}

		_payload_[_length_++] = 0x04 | (mod << 6);
		_D(ZASM_LOG_WARNING, "Mod R/W       - %02X", _payload_[_length_-1]);

		mod = 0x0;
		rm  = this->dst().isMEM2() ? this->dst().asInt() : this->src().asInt();
		reg = token->asInt();

		_payload_[_length_++] = (mod & 0x3) << 6 | (reg & 0x7) << 3 | (rm & 0x7);
		_D(ZASM_LOG_WARNING, "SIB           - %02X", _payload_[_length_-1]);
		delete token;
	} else if ((this->dst().isMEM() || this->src().isMEM()) || (inst.flags & INST_REG_OPERANDS)) {
		reg = this->dst().isREG() ? this->dst().asInt() :
					(this->src().isREF() ? 0 : this->src().asInt());
		if (this->src().isIMM() && ! this->src().isREF()) reg = 0;

		if (this->offset()) {
			mod = 0 == (~0x7F & this->offset()) ? 0x01 : 0x02;
		} else if (this->src().isMEM() && !this->src().isREF()) {
			ZasmToken *base = this->src().asReg();

			mod = (*base == "rsp" || *base == "rbp") ? 0x01 : 0x00;
		} else if (this->dst().isMEM() && !this->dst().isREF()) {
			ZasmToken *base = this->dst().asReg();

			mod = (*base == "rsp" || *base == "rbp") ? 0x01 : 0x00;
		}


		if (! this->dst().isMEM() && this->src().isREF()) {
			_D(ZASM_LOG_INFO, "EIP RIP or R13 ");
			rm = 0x05;	/* EIP / RIP / R13 */
		} else if (this->dst().isMEM() || this->src().isMEM()) {
			rm = this->dst().isMEM() ? this->dst().asInt() : this->src().asInt();
		}

		if (this->dst().isREG() && 1 == this->dst().size() && 'h' == this->dst().raw()[1]) {
			reg |= 0x04;
		} else if (this->src().isREG() && 1 == this->src().size() && 'h' == this->src().raw()[1]) {
			reg |= 0x04;
		}

		_payload_[_length_++] = (mod & 0x3) << 6 | (reg & 0x7) << 3 | (rm & 0x7);
		_D(ZASM_LOG_WARNING, "Mod R/W       - %02X", _payload_[_length_-1]);

		if (mod == 0x01) {
			_payload_[_length_++] = 0x0;
		}
	}
}
void Instruction::displacement(X86_64_INST &inst) {
	off_t ret;
	ZasmToken token;

	if (this->dst().isMEM() && this->dst().offset()) {
		token = this->dst();
	} else if (this->src().isMEM() && this->src().offset()) {
		token = this->src();
	} else {
		return;
	}

	ret = this->setIMM(token.offset(), 0 == (~0x7F & token.offset()) ? 1 : 4);
	if (ret) _D(ZASM_LOG_WARNING, "Displacement  - %llX", ret);
}
void Instruction::immediate(X86_64_INST &inst) {
	off_t ret = 0;
	int size = 4;

	if (this->src().isREF()) {
		ret = this->setIMM(-1, size);
	} else if (this->dst().isIMM() || this->src().isIMM()) {
		ZasmToken token = this->dst().isIMM() ? this->dst() : this->src();

		size = token.size();
		{	/* FIXME - Need to be more logical */
			size = size == 2 ? 4 : size;
			if (0xB8 == inst.opcode && 1 == size) size = 4;
			if (0xF7 == inst.opcode && 1 == size) size = 4;
			if (this->dst().isMEM() && 1 == size) size = 4;
			if (this->dst().isIMM() && 1 == size) size = 4;
		}
		ret  = this->setIMM(token.asInt(), size);
	}
	if (ret) _D(ZASM_LOG_WARNING, "Immediate     - %llX", ret);
}
#endif /* __x86_64__ */
