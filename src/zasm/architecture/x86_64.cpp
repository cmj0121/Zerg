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
	/* legacy prefix - group 1 (LOCK/REPNE/REP)          */
	if (inst.flags & INST_PF_LOCK) {
		_payload_[_length_++] = 0xF0;
	}
	/* legacy prefix - group 2 (segment register/branch) */
	if (this->dst.isSEGReg() || this->src.isSEGReg()) {
		_D(LOG_CRIT, "Not implemented");
	}
	/* legacy prefix - group 3 (operand-size override)   */
	/* legacy prefix - group 4 (address-size override)   */
}
void Instruction::opcode(X86_64_INST &inst, int mode) {
	/* NOTE  - Opcode        (1~2) */
	if (INST_PF_TWOBYTE & inst.flags) {
		_D(LOG_ZASM_INFO, "Two Byte      - 0F");
		_payload_[_length_++] = 0x0F;
	}

	/* set opcode */
	_payload_[_length_++] = inst.opcode;

	if ((inst.flags & INST_SECODE_OP) && this->dst) {
		_payload_[_length_-1] |= this->dst.asInt();
	}
	_D(LOG_ZASM_INFO, "Opcode        - %02X", inst.opcode);
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
}
void Instruction::displacement(X86_64_INST &inst, int mode) {
}
void Instruction::immediate(X86_64_INST &inst, int mode) {
}

#endif /* __x86_64__ */
