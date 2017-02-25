/* Copyright (C) 2014-2016 cmj. All right reserved. */
#ifndef __ZASM_X86_64_INSTRUCT_H__
#  define __ZASM_X86_64_INSTRUCT_H__

INST_ASM_OP0(nop, 0x90)
INST_ASM_OP0(cdq, 0x99)
INST_ASM_OP (cqo, 0x99, INST_NONE, INST_NONE, INST_EXTENSION_64)
INST_ASM_OP0(ret, 0xC3)

INST_ASM_OP (add, 0x01, INST_REG_MEM16_32_64,	INST_REG16_32_64,		INST_REG_OPERANDS)
INST_ASM_OP (add, 0x03, INST_REG_MEM16_32_64,	INST_REG_MEM16_32_64,	INST_REG_OPERANDS)
INST_ASM_OP (or,  0x09, INST_REG_MEM16_32_64,   INST_REG16_32_64,       INST_REG_OPERANDS)
INST_ASM_OP (or,  0x0A, INST_REG8,              INST_REG8 | INST_MEM8,  INST_REG_OPERANDS)
INST_ASM_OP (or,  0x0B, INST_REG16_32_64,       INST_REG_MEM16_32_64,   INST_REG_OPERANDS)
INST_ASM_OP (and, 0x21, INST_REG_MEM16_32_64,   INST_REG16_32_64,       INST_REG_OPERANDS)
INST_ASM_OP (and, 0x22, INST_REG8,              INST_REG8 | INST_MEM8,  INST_REG_OPERANDS)
INST_ASM_OP (and, 0x23, INST_REG16_32_64,       INST_REG_MEM16_32_64,   INST_REG_OPERANDS)
INST_ASM_OP (sub, 0x2B, INST_REG_MEM16_32_64,	INST_MEM16_32_64,		INST_REG_OPERANDS)
INST_ASM_OP (sub, 0x2A, INST_REG8 | INST_MEM8,	INST_MEM8,				INST_REG_OPERANDS)
INST_ASM_OP (sub, 0x29, INST_REG_MEM16_32_64,	INST_REG16_32_64,		INST_REG_OPERANDS)

INST_ASM_OP (push, 0x50, INST_REG16 | INST_REG64, INST_NONE, INST_SECONDARY)
INST_ASM_OP (pop,  0x58, INST_REG16 | INST_REG64, INST_NONE, INST_SECONDARY)

INST_ASM_OP1(push, 0x68, INST_IMM16_32)
INST_ASM_OP1(call, 0xE8, INST_IMM16_32)
INST_ASM_OP1(jmp,  0xE9, INST_IMM16_32)

INST_ASM_OP (xor, 0x31, INST_REG_MEM16_32_64,   INST_REG16_32_64,       INST_REG_OPERANDS)
INST_ASM_OP (xor, 0x32, INST_REG8,              INST_REG8 | INST_MEM8,  INST_REG_OPERANDS)
INST_ASM_OP (xor, 0x33, INST_REG16_32_64,       INST_REG_MEM16_32_64,   INST_REG_OPERANDS)
INST_ASM_OP2(cmp, 0x38, INST_REG8 | INST_MEM8, INST_REG8 | INST_MEM8)
INST_ASM_OP2(cmp, 0x39, INST_REG_MEM16_32_64, INST_REG_MEM16_32_64)

INST_ASM_OP (mul, 0x6B, INST_REG16_32_64,		INST_IMM8,		INST_OPCODE_EXT | INST_OPCODE_MULTI)
INST_ASM_OP (mul, 0x69, INST_REG16_32_64,		INST_IMM16_32,	INST_OPCODE_EXT | INST_OPCODE_MULTI)
INST_ASM_OP (add, 0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x00)
INST_ASM_OP (or,  0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x01)
INST_ASM_OP (adc, 0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x02)
INST_ASM_OP (sbb, 0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x03)
INST_ASM_OP (and, 0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x04)
INST_ASM_OP (sub, 0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x05)
INST_ASM_OP (xor, 0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x06)
INST_ASM_OP (cmp, 0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x07)

INST_ASM_OP (add, 0x81, INST_REG_MEM16_32_64, INST_IMM16_32, INST_OPCODE_EXT | 0x00)
INST_ASM_OP (or,  0x81, INST_REG_MEM16_32_64, INST_IMM16_32, INST_OPCODE_EXT | 0x01)
INST_ASM_OP (adc, 0x81, INST_REG_MEM16_32_64, INST_IMM16_32, INST_OPCODE_EXT | 0x02)
INST_ASM_OP (sbb, 0x81, INST_REG_MEM16_32_64, INST_IMM16_32, INST_OPCODE_EXT | 0x03)
INST_ASM_OP (and, 0x81, INST_REG_MEM16_32_64, INST_IMM16_32, INST_OPCODE_EXT | 0x04)
INST_ASM_OP (sub, 0x81, INST_REG_MEM16_32_64, INST_IMM16_32, INST_OPCODE_EXT | 0x05)
INST_ASM_OP (xor, 0x81, INST_REG_MEM16_32_64, INST_IMM16_32, INST_OPCODE_EXT | 0x06)
INST_ASM_OP (cmp, 0x81, INST_REG_MEM16_32_64, INST_IMM16_32, INST_OPCODE_EXT | 0x07)

INST_ASM_OP (add, 0x80, INST_REG8 | INST_MEM8, INST_IMM8, INST_OPCODE_EXT | 0x00)
INST_ASM_OP (or,  0x80, INST_REG8 | INST_MEM8, INST_IMM8, INST_OPCODE_EXT | 0x01)
INST_ASM_OP (adc, 0x80, INST_REG8 | INST_MEM8, INST_IMM8, INST_OPCODE_EXT | 0x02)
INST_ASM_OP (sbb, 0x80, INST_REG8 | INST_MEM8, INST_IMM8, INST_OPCODE_EXT | 0x03)
INST_ASM_OP (and, 0x80, INST_REG8 | INST_MEM8, INST_IMM8, INST_OPCODE_EXT | 0x04)
INST_ASM_OP (sub, 0x80, INST_REG8 | INST_MEM8, INST_IMM8, INST_OPCODE_EXT | 0x05)
INST_ASM_OP (xor, 0x80, INST_REG8 | INST_MEM8, INST_IMM8, INST_OPCODE_EXT | 0x06)
INST_ASM_OP (cmp, 0x80, INST_REG8 | INST_MEM8, INST_IMM8, INST_OPCODE_EXT | 0x07)

INST_ASM_OP (test, 0x85, INST_REG_MEM16_32_64,	INST_REG16_32_64, INST_REG_OPERANDS)
INST_ASM_OP (test, 0x84, INST_REG8 | INST_MEM8,	INST_REG8,        INST_REG_OPERANDS)
INST_ASM_OP (xchg, 0x86, INST_REG8, INST_REG8 | INST_MEM8,        INST_REG_OPERANDS)
INST_ASM_OP (xchg, 0x87, INST_REG16_32_64, INST_REG_MEM16_32_64,  INST_REG_OPERANDS)

INST_ASM_OP (mov, 0x89, INST_REG_MEM16_32_64,	INST_REG16_32_64, 	INST_REG_OPERANDS)
INST_ASM_OP (mov, 0x88, INST_REG8 | INST_MEM8,	INST_REG_ALL,		INST_REG_OPERANDS)
INST_ASM_OP (mov, 0x8B, INST_REG16_32_64,		INST_MEM16_32_64,	INST_REG_OPERANDS)
INST_ASM_OP (mov, 0x8A, INST_REG_ALL,			INST_MEM8,			INST_REG_OPERANDS)

INST_ASM_OP2(lea, 0x8D, INST_REG16_32_64, INST_MEM | INST_SIZE_ALL)
INST_ASM_OP (mov, 0xB8, INST_REG16_32_64, INST_IMM16_32_64,		INST_SECONDARY)

INST_ASM_OP (rol,  0xC0, INST_REG8 | INST_MEM8, INST_IMM8,		INST_OPCODE_EXT | 0x0)
INST_ASM_OP (ror,  0xC0, INST_REG8 | INST_MEM8, INST_IMM8,		INST_OPCODE_EXT | 0x1)
INST_ASM_OP (rcl,  0xC0, INST_REG8 | INST_MEM8, INST_IMM8,		INST_OPCODE_EXT | 0x2)
INST_ASM_OP (rcr,  0xC0, INST_REG8 | INST_MEM8, INST_IMM8,		INST_OPCODE_EXT | 0x3)
INST_ASM_OP (shl,  0xC0, INST_REG8 | INST_MEM8, INST_IMM8,		INST_OPCODE_EXT | 0x4)
INST_ASM_OP (shr,  0xC0, INST_REG8 | INST_MEM8, INST_IMM8,		INST_OPCODE_EXT | 0x5)
INST_ASM_OP (sal,  0xC0, INST_REG8 | INST_MEM8, INST_IMM8,		INST_OPCODE_EXT | 0x4)	// same as shl
INST_ASM_OP (sar,  0xC0, INST_REG8 | INST_MEM8, INST_IMM8,		INST_OPCODE_EXT | 0x7)

INST_ASM_OP (rol,  0xC1, INST_REG_MEM16_32_64, INST_IMM8,		INST_OPCODE_EXT | 0x0)
INST_ASM_OP (ror,  0xC1, INST_REG_MEM16_32_64, INST_IMM8,		INST_OPCODE_EXT | 0x1)
INST_ASM_OP (rcl,  0xC1, INST_REG_MEM16_32_64, INST_IMM8,		INST_OPCODE_EXT | 0x2)
INST_ASM_OP (rcr,  0xC1, INST_REG_MEM16_32_64, INST_IMM8,		INST_OPCODE_EXT | 0x3)
INST_ASM_OP (shl,  0xC1, INST_REG_MEM16_32_64, INST_IMM8,		INST_OPCODE_EXT | 0x4)
INST_ASM_OP (shr,  0xC1, INST_REG_MEM16_32_64, INST_IMM8,		INST_OPCODE_EXT | 0x5)
INST_ASM_OP (sal,  0xC1, INST_REG_MEM16_32_64, INST_IMM8,		INST_OPCODE_EXT | 0x4)	// same as shl
INST_ASM_OP (sar,  0xC1, INST_REG_MEM16_32_64, INST_IMM8,		INST_OPCODE_EXT | 0x7)

INST_ASM_OP (mov,  0xC7, INST_REG_MEM16_32_64, INST_IMM16_32,	INST_REG_OPERANDS)
INST_ASM_OP (mov,  0xC6, INST_REG8 | INST_MEM8, INST_IMM8,		INST_REG_OPERANDS)

INST_ASM_OP (rol,  0xD2, INST_REG8 | INST_MEM8, INST_REG_CL,	INST_OPCODE_EXT | 0x0)
INST_ASM_OP (ror,  0xD2, INST_REG8 | INST_MEM8, INST_REG_CL,	INST_OPCODE_EXT | 0x1)
INST_ASM_OP (rcl,  0xD2, INST_REG8 | INST_MEM8, INST_REG_CL,	INST_OPCODE_EXT | 0x2)
INST_ASM_OP (rcr,  0xD2, INST_REG8 | INST_MEM8, INST_REG_CL,	INST_OPCODE_EXT | 0x3)
INST_ASM_OP (shl,  0xD2, INST_REG8 | INST_MEM8, INST_REG_CL,	INST_OPCODE_EXT | 0x4)
INST_ASM_OP (shr,  0xD2, INST_REG8 | INST_MEM8, INST_REG_CL,	INST_OPCODE_EXT | 0x5)
INST_ASM_OP (sal,  0xD2, INST_REG8 | INST_MEM8, INST_REG_CL,	INST_OPCODE_EXT | 0x4)	// same as shl
INST_ASM_OP (sar,  0xD2, INST_REG8 | INST_MEM8, INST_REG_CL,	INST_OPCODE_EXT | 0x7)

INST_ASM_OP (rol,  0xD3, INST_REG_MEM16_32_64, INST_REG_CL,	INST_OPCODE_EXT | 0x0)
INST_ASM_OP (ror,  0xD3, INST_REG_MEM16_32_64, INST_REG_CL,	INST_OPCODE_EXT | 0x1)
INST_ASM_OP (rcl,  0xD3, INST_REG_MEM16_32_64, INST_REG_CL,	INST_OPCODE_EXT | 0x2)
INST_ASM_OP (rcr,  0xD3, INST_REG_MEM16_32_64, INST_REG_CL,	INST_OPCODE_EXT | 0x3)
INST_ASM_OP (shl,  0xD3, INST_REG_MEM16_32_64, INST_REG_CL,	INST_OPCODE_EXT | 0x4)
INST_ASM_OP (shr,  0xD3, INST_REG_MEM16_32_64, INST_REG_CL,	INST_OPCODE_EXT | 0x5)
INST_ASM_OP (sal,  0xD3, INST_REG_MEM16_32_64, INST_REG_CL,	INST_OPCODE_EXT | 0x4)	// same as shl
INST_ASM_OP (sar,  0xD3, INST_REG_MEM16_32_64, INST_REG_CL,	INST_OPCODE_EXT | 0x7)

INST_ASM_OP (test, 0xF7, INST_REG_MEM16_32_64, INST_IMM16_32,	INST_OPCODE_EXT | 0x0)
INST_ASM_OP (not,  0xF7, INST_REG_MEM16_32_64, INST_NONE,		INST_OPCODE_EXT | 0x2)
INST_ASM_OP (neg,  0xF7, INST_REG_MEM16_32_64, INST_NONE,		INST_OPCODE_EXT | 0x3)
INST_ASM_OP (div,  0xF7, INST_REG_MEM16_32_64, INST_NONE,		INST_OPCODE_EXT | 0x6)
INST_ASM_OP (idiv,  0xF7, INST_REG_MEM16_32_64, INST_NONE,		INST_OPCODE_EXT | 0x7)

INST_ASM_OP (inc,  0xFE, INST_REG8 | INST_MEM8, INST_NONE,		INST_OPCODE_EXT | 0x0)
INST_ASM_OP (dec,  0xFE, INST_REG8 | INST_MEM8, INST_NONE,		INST_OPCODE_EXT | 0x1)
INST_ASM_OP (inc,  0xFF, INST_REG16_32 | INST_MEM16_32, INST_NONE,	INST_OPCODE_EXT | 0x0)
INST_ASM_OP (dec,  0xFF, INST_REG16_32 | INST_MEM16_32, INST_NONE,	INST_OPCODE_EXT | 0x1)
INST_ASM_OP (call, 0xFF, INST_REG16_32 | INST_MEM16_32,	INST_NONE,	INST_OPCODE_EXT | 0x2)
INST_ASM_OP (push, 0xFF, INST_REG16_32_64 | INST_MEM16_32_64,	INST_NONE,	INST_OPCODE_EXT | 0x6)


INST_ASM_OP_TWOBYTE(syscall, 0x05, INST_NONE,			INST_NONE,	INST_NONE)

INST_ASM_OP_TWOBYTE(je,    0x84, INST_IMM16_32,         INST_NONE,  INST_NONE)
INST_ASM_OP_TWOBYTE(jz,    0x84, INST_IMM16_32,         INST_NONE,  INST_NONE)
INST_ASM_OP_TWOBYTE(jne,   0x85, INST_IMM16_32,         INST_NONE,  INST_NONE)
INST_ASM_OP_TWOBYTE(jbe,   0x86, INST_IMM16_32,         INST_NONE,  INST_NONE)
INST_ASM_OP_TWOBYTE(jnbe,  0x87, INST_IMM16_32,         INST_NONE,  INST_NONE)
INST_ASM_OP_TWOBYTE(js,    0x88, INST_IMM16_32,         INST_NONE,  INST_NONE)
INST_ASM_OP_TWOBYTE(jns,   0x89, INST_IMM16_32,         INST_NONE,  INST_NONE)
INST_ASM_OP_TWOBYTE(jpe,   0x8A, INST_IMM16_32,         INST_NONE,  INST_NONE)
INST_ASM_OP_TWOBYTE(jpo,   0x8B, INST_IMM16_32,         INST_NONE,  INST_NONE)
INST_ASM_OP_TWOBYTE(jl,    0x8C, INST_IMM16_32,         INST_NONE,  INST_NONE)
INST_ASM_OP_TWOBYTE(jge,   0x8D, INST_IMM16_32,         INST_NONE,  INST_NONE)
INST_ASM_OP_TWOBYTE(jle,   0x8E, INST_IMM16_32,         INST_NONE,  INST_NONE)
INST_ASM_OP_TWOBYTE(jg,    0x8F, INST_IMM16_32,         INST_NONE,  INST_NONE)

INST_ASM_OP_TWOBYTE(setl,  0x9C, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setge, 0x9D, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)

INST_ASM_OP_TWOBYTE(seto,   0x90, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setno,  0x91, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setc,   0x92, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setnc,  0x93, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setz,   0x94, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setnz,  0x95, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setcz,  0x96, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setncz, 0x97, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(sets,   0x98, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setns,  0x99, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setp,   0x9A, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setnp,  0x9B, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setl,   0x9C, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setnl,  0x9D, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setng,  0x9E, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setg,   0x9F, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)

INST_ASM_OP_TWOBYTE(setle, 0x9E, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(setg,  0x9F, INST_MEM8 | INST_REG8, INST_NONE,  INST_OPCODE_EXT)
INST_ASM_OP_TWOBYTE(mul,   0xAF, INST_REG16_32_64,		INST_REG_MEM16_32_64,	INST_REG_OPERANDS | INST_REG_SWAP)

/* SSE (Streaming SIMD Extensions) Instruction */
INST_ASM_OP_TWOBYTE(movss, 0x10, INST_XMM,	INST_XMM | INST_MEM32, INST_SINGLE_FP | INST_OPCODE_MULTI)
INST_ASM_OP_TWOBYTE(movsd, 0x10, INST_XMM,	INST_XMM | INST_MEM64, INST_DOUBLE_FP | INST_OPCODE_MULTI)

#endif /* __ZASM_X86_64_INSTRUCT_H__ */

