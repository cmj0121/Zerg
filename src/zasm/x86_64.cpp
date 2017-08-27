#ifdef __x86_64__
/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include <sstream>
#include <string.h>
#include "zasm.h"

#include "zasm/x86_64_inst.h"

/* X86-64 Instruction Encoding - Max to 15 bits
 *
 * 0               4        6     7     8                         16
 * |---------------|--------|-----|-----|--------------------------|
 *   Legacy Prefix   OpCode   Mod   SIB   Displacement / Immediate
 */

void Instruction::legacyPrefix(X86_64_INST &inst, int mode) {
	bool blDWORD = false;

	/* NOTE - Legacy Prefix (0~4) */
	if (0 != (inst.flags & INST_SINGLE_FP)) {
		/* single float */
		_payload_[_length_++] = 0xF3;
	}
	if (0 != (inst.flags & INST_DOUBLE_FP)) {
		/* double float */
		_payload_[_length_++] = 0xF2;
	} else if (0 != (inst.flags & INST_EXTENSION_64)) {
		/* force 64-bit extension */
		_payload_[_length_++] = 0x48;
	}

	if (X86_REAL_MODE != mode && (CPU_16BIT == this->src.size() ||
		(CPU_16BIT == this->dst.size() && !this->src.isNULL()))) {
		/* 16-bit memory access */
		_payload_[_length_++] = 0x66;
		blDWORD = true;
	} else if ((this->src.isMEM() && CPU_32BIT == this->src.size() && !this->src.isEXT()) ||
			(this->dst.isMEM() && CPU_32BIT == this->dst.size() && !this->dst.isEXT())) {
		/* 32-bit memory access */
		return ;
	}

	if (CPU_64BIT == this->dst.size() || CPU_64BIT == this->src.size() ||
		this->src.isEXT() || this->dst.isEXT() ||
		(this->src.isMEM2() && this->src.indexReg()->isEXT()) ||
		(this->dst.isMEM2() && this->dst.indexReg()->isEXT())) {
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
		if (this->dst.isREG() && (this->src.isREG() || this->src.isIMM())) {
			if (this->dst.isEXT())
				REX_B = 1;
		} else if (this->dst.isREG() && "" == this->src.raw()) {
			if (this->dst.isEXT())
				REX_B = 1;
		} else if (this->dst.isMEM() && this->dst.isEXT()) {
			REX_B = 1;
		} else if (this->src.isMEM() && this->dst.isREG() &&this->src.isEXT()) {
			REX_B = 1;
		}

		if (this->src.isREG() && this->dst.isREG()) {
			if (this->src.isEXT())
				REX_R = 1;
		} else if (this->src.isMEM() && this->dst.isEXT()) {
			REX_R = 1;
		} else if (this->dst.isMEM() && this->src.isREG() && this->src.isEXT()) {
			REX_R = 1;
		}

		if (this->dst.isMEM2() && this->dst.indexReg()->isEXT()) {
			REX_X = 1;
		} else if (this->src.isMEM2() && this->src.indexReg()->isEXT()) {
			REX_X = 1;
		}

		if (0 == (inst.flags & INST_SECONDARY) && false == blDWORD) {
			REX_W = 1;
		}

		/* HACK - Do NOT know why, but multiple is strange in legacy  */
		if (0 == strncmp("mul", inst.cmd, 3) && 0xAF == inst.opcode) {
			REX_R = REX_R ^ REX_B;
			REX_B = REX_R ^ REX_B;
			REX_R = REX_R ^ REX_B;
		}


		if (REX_B || REX_X || REX_R || REX_W) {
			_payload_[_length_++] = 0x40 | (REX_W << 3) | (REX_R << 2) | (REX_X << 1) | REX_B;
			_D(LOG_ZASM_INFO, "Legacy Prefix - %02X", _payload_[_length_-1]);
		}
	}
}
void Instruction::opcode(X86_64_INST &inst, int mode) {
	/* NOTE  - Opcode        (1~2) */
	if (INST_TWO_BYTE & inst.flags) {
		_D(LOG_ZASM_INFO, "Two Byte      - 0F");
		_payload_[_length_++] = 0x0F;
	}
	_payload_[_length_++] = inst.opcode;

	if (inst.flags & INST_SECONDARY && this->dst) {
		_payload_[_length_-1] |= this->dst.asInt();
	}
	_D(LOG_ZASM_INFO, "Opcode        - %02X", inst.opcode);

	/* NOTE - Secondary Opcode  or Opcode Extension */
	if (inst.flags & INST_OPCODE_EXT) {
		_payload_[_length_++] = (this->dst.isREG() ? 0x03 : 0x0) << 6;

		if (this->dst.isMEM() && this->dst.isMEM2()) {
			_payload_[_length_-1] |= 0x14;
		}
		if (this->dst.isMEM() && this->dst.offset()) {
			_payload_[_length_-1] |= 0x40;
		}

		switch(this->dst.asInt()) {
			case 4: case 5:
				_payload_[_length_-1] |= 0x40;
			default:
				if (0x54 != _payload_[_length_-1])
				_payload_[_length_-1] |= this->dst.asInt();
				break;
		}
		if (INST_OPCODE_MULTI & inst.flags) {
			_payload_[_length_-1] |= this->dst.asInt()  << 3;	// Extension
		} else {
			_payload_[_length_-1] |= (inst.flags & 0xF) << 3;	// Extension
		}
		_D(LOG_ZASM_INFO, "Opcode Ext    - %02X", _payload_[_length_-1]);
	}
}
void Instruction::modRW(X86_64_INST &inst, int mode) {
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

	if (! this->dst || ! this->src || (INST_REG_SPECIFY & inst.op2)) {
		return ;
	} else if (this->dst.isREG() && this->src.isREG()) {
		mod = 0x03;
		reg = this->src.asInt();
		rm  = this->dst.asInt();

		/* HACK */
		if ((this->dst.isSSE() && !this->src.isSSE()) ||
			(this->src.isSSE() && !this->dst.isSSE())) {
			mod = 0x0;
		}

		if (inst.flags & INST_REG_SWAP) {
			reg ^= rm;
			rm  ^= reg;
			reg ^= rm;
		}

		_payload_[_length_++] = (mod & 0x3) << 6 | (reg & 0x7) << 3 | (rm & 0x7);
		_D(LOG_ZASM_INFO, "Mod R/W       - %02X", _payload_[_length_-1]);
	} else if (this->dst.isMEM2() || this->src.isMEM2()) {
		InstToken *token = this->dst.isMEM2() ? this->dst.indexReg() : this->src.indexReg();
		off_t offset = this->dst.offset() ? this->dst.offset() : this->src.offset();

		if (offset) {
			mod = 0 == (~0x7F & offset) ? 0x01 : 0x02;
		}

		/* NOTE - reg is the register position, not always dst or src */
		reg = this->dst.isREG() ? this->dst.asInt() :
						(this->src.isREG() ? this->src.asInt(): 0x0 );
		_payload_[_length_++] = 0x04 | (reg & 0x07) << 3 | (mod << 6);
		_D(LOG_ZASM_INFO, "Mod R/W       - %02X", _payload_[_length_-1]);

		mod = 0x0;
		rm  = this->dst.isMEM2() ? this->dst.asInt() : this->src.asInt();
		reg = token->asInt();

		_payload_[_length_++] = (mod & 0x3) << 6 | (reg & 0x7) << 3 | (rm & 0x7);
		_D(LOG_ZASM_INFO, "SIB           - %02X", _payload_[_length_-1]);
		delete token;
	} else if ((this->dst.isMEM() || this->src.isMEM()) || (inst.flags & INST_REG_OPERANDS)) {
		reg = this->dst.isREG() ? this->dst.asInt() :
					(this->src.isREF() ? 0 : this->src.asInt());
		if (this->src.isIMM() && ! this->src.isREF()) reg = 0;

		if (this->offset()) {
			mod = 0 == (~0x7F & this->offset()) ? 0x01 : 0x02;
		} else if (this->src.isMEM() && !this->src.isREF()) {
			InstToken *base = this->src.asReg();

			mod = base->isPosREG() && 4 != base->asInt() % 8 ? 0x01 : 0x00;
		} else if (this->dst.isMEM() && !this->dst.isREF()) {
			InstToken *base = this->dst.asReg();

			mod = base->isPosREG() && 4 != base->asInt() % 8 ? 0x01 : 0x00;
		}


		if (! this->dst.isMEM() && this->src.isREF()) {
			_D(LOG_ZASM_INFO, "EIP RIP or R13 ");
			rm = 0x05;	/* EIP / RIP / R13 */
		} else if (this->dst.isMEM() || this->src.isMEM()) {
			rm = this->dst.isMEM() ? this->dst.asInt() : this->src.asInt();
		}

		if (this->dst.isREG() && CPU_8BIT == this->dst.size() && 'h' == this->dst.raw()[1]) {
			reg |= 0x04;
		} else if (this->src.isREG() && CPU_8BIT == this->src.size() && 'h' == this->src.raw()[1]) {
			reg |= 0x04;
		}

		_payload_[_length_++] = (mod & 0x3) << 6 | (reg & 0x7) << 3 | (rm & 0x7);
		_D(LOG_ZASM_INFO, "Mod R/W       - %02X", _payload_[_length_-1]);

		if ((mod == 0x01 && NULL != this->src.indexReg())
			|| (this->src.isMEM() && 5 == this->src.asInt() % 8)
			|| (this->dst.isMEM() && 5 == this->dst.asInt() % 8)) {

			if (0xFF < this->offset() && 0 == (~0x7F & this->offset())) {
				_payload_[_length_++] = 0x0;
				_D(LOG_ZASM_INFO, "Mod R/W       - %02X", _payload_[_length_-1]);
			}
		} else if ((this->src.isMEM() && 4 == this->src.asInt() % 8) ||
					(this->dst.isMEM() && 4 == this->dst.asInt() % 8)) {
			_payload_[_length_++] = 0x24;
			_D(LOG_ZASM_INFO, "Mod R/W       - %02X", _payload_[_length_-1]);
		}
	}
}
void Instruction::displacement(X86_64_INST &inst, int mode) {
	off_t ret;
	InstToken token;

	if (this->src.isREF()) {
		return ;
	} else if (this->dst.isMEM() && this->dst.offset()) {
		token = this->dst;
	} else if (this->src.isMEM() && this->src.offset()) {
		token = this->src;
	} else if (this->src.isMEM() && this->src.asReg()->isPosREG()) {
		token = this->src;
	} else {
		return;
	}

	ret = token.offset();
	ret = (0 > ret && INST_NONE == inst.op2) ? ~ret : ret;
	ret = this->setIMM(token.offset(), 0 == (~0x7F & ret) ? 1 : 4);
	if (ret) _D(LOG_ZASM_INFO, "Displacement  - " OFF_T, ret);
}
void Instruction::immediate(X86_64_INST &inst, int mode) {
	off_t ret = 0;
	int size = 4;

	if (this->src.isREF()) {
		ret = this->setIMM(-1, size);
		_D(LOG_ZASM_INFO, "Immediate     - reference");
	} else if (this->dst.isIMM() || this->src.isIMM()) {
		switch(inst.opcode) {
			case 0xB8:
			case 0xF7:
				size = this->src.size();
				size = 1 == size ? (X86_REAL_MODE == mode ? CPU_16BIT : CPU_32BIT) : size;
				break;
			default:
				size = this->dst.isIMM() ? this->dst.size() : this->src.size();
				break;
		}

		switch((this->dst.isIMM() ? inst.op1 : inst.op2) & INST_SIZE_ALL) {
			case INST_SIZE8:
				size = CPU_8BIT;
				break;
			default:
				size = (CPU_8BIT == size || CPU_64BIT == size) ? CPU_32BIT : size;
				size = (CPU_16BIT == size && this->src.isNULL()) ? CPU_32BIT : size;
				break;
		}

		ret  = this->dst.isIMM() ? this->dst.asInt() : this->src.asInt();
		ret  = this->setIMM(ret, size);

		/* check the immediate size in real mode */
		if (size > CPU_16BIT && X86_REAL_MODE == mode) {
			_D(LOG_CRIT, "in real mode, immediate too long " OFF_T, ret);
		}
		_D(LOG_ZASM_INFO, "Immediate (%d) - " OFF_T, size, ret);
	}
}

#endif /* __x86_64__ */
