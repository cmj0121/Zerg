/* Copyright (C) 2014-2016 cmj. All right reserved. */
#ifndef __ZASM_X86_64_INSTRUCT_H__
#  define __ZASM_X86_64_INSTRUCT_H__

#define INST_SIZE16_32			INST_SIZE16 | INST_SIZE32
#define INST_SIZE16_32_64		INST_SIZE16 | INST_SIZE32 | INST_SIZE64

#define INST_REG8				INST_REG | INST_SIZE8
#define INST_MEM8				INST_MEM | INST_SIZE8
#define INST_IMM8				INST_IMM | INST_SIZE8
#define INST_REF8				INST_REF | INST_SIZE8
#define INST_REG_MEM8			INST_REG | INST_MEM | INST_SIZE8

#define INST_MEMALL				INST_MEM | INST_SIZE_ALL

#define INST_REG16_32			INST_REG | INST_SIZE16_32
#define INST_MEM16_32			INST_MEM | INST_SIZE16_32
#define INST_IMM16_32			INST_IMM | INST_SIZE16_32
#define INST_REF16_32			INST_REF | INST_SIZE16_32

#define INST_REG16_32_64		INST_REG | INST_SIZE16_32_64

#define INST_REG16_64			INST_REG | INST_SIZE16 | INST_SIZE64
#define INST_MEM16_64			INST_MEM | INST_SIZE16 | INST_SIZE64
#define INST_REG_MEM16_64		INST_REG | INST_MEM | INST_SIZE16 | INST_SIZE64

#define INST_REG_MEM16_32		INST_REG | INST_MEM | INST_SIZE16_32
#define INST_REG_MEM16_32_64	INST_REG | INST_MEM | INST_SIZE16_32_64

_ASM_OP (add, 0x00, INST_REG_MEM8,			INST_REG8,				INST_FLD_REG)
_ASM_OP (add, 0x01, INST_REG_MEM16_32_64,	INST_REG16_32_64,		INST_FLD_REG)
_ASM_OP (add, 0x02, INST_REG8,				INST_REG_MEM8,			INST_FLD_REG)
_ASM_OP (add, 0x03, INST_REG_MEM16_32_64,	INST_REG_MEM16_32_64,	INST_FLD_REG)
_ASM_OP2(add, 0x04, INST_REG_AL,			INST_IMM8)
_ASM_OP2(add, 0x05, INST_REG_RAX,			INST_IMM16_32)
/* 0x06 - invalid */
/* 0x07 - invalid */
_ASM_OP (or,  0x08, INST_REG_MEM8,			INST_REG8,				INST_FLD_REG)
_ASM_OP (or,  0x09, INST_REG_MEM16_32_64,   INST_REG16_32_64,       INST_FLD_REG)
_ASM_OP (or,  0x0A, INST_REG8,              INST_REG_MEM8, 			INST_FLD_REG)
_ASM_OP (or,  0x0B, INST_REG16_32_64,       INST_REG_MEM16_32_64,   INST_FLD_REG)
_ASM_OP2(or,  0x0C, INST_REG_AL,			INST_IMM8)
_ASM_OP2(or,  0x0D, INST_REG_RAX,			INST_IMM16_32)
/* 0x0E - invalid */
/* 0x0F - two-byte prefix */
_ASM_OP (adc, 0x10, INST_REG_MEM8,			INST_REG8,				INST_FLD_REG)
_ASM_OP (adc, 0x11, INST_REG_MEM16_32_64,   INST_REG16_32_64,       INST_FLD_REG)
_ASM_OP (adc, 0x12, INST_REG8,              INST_REG_MEM8, 			INST_FLD_REG)
_ASM_OP (adc, 0x13, INST_REG16_32_64,       INST_REG_MEM16_32_64,   INST_FLD_REG)
_ASM_OP2(adc, 0x14, INST_REG_AL,			INST_IMM8)
_ASM_OP2(adc, 0x15, INST_REG_RAX,			INST_IMM16_32)
/* 0x16 - invalid */
/* 0x17 - invalid */
_ASM_OP (sbb, 0x18, INST_REG_MEM8,			INST_REG8,				INST_FLD_REG)
_ASM_OP (sbb, 0x19, INST_REG_MEM16_32_64,   INST_REG16_32_64,       INST_FLD_REG)
_ASM_OP (sbb, 0x1A, INST_REG8,              INST_REG_MEM8, 			INST_FLD_REG)
_ASM_OP (sbb, 0x1B, INST_REG16_32_64,       INST_REG_MEM16_32_64,   INST_FLD_REG)
_ASM_OP2(sbb, 0x1C, INST_REG_AL,			INST_IMM8)
_ASM_OP2(sbb, 0x1D, INST_REG_RAX,			INST_IMM16_32)
/* 0x1E - invalid */
/* 0x1F - invalid */
_ASM_OP (and, 0x20, INST_REG_MEM8,			INST_REG8,				INST_FLD_REG)
_ASM_OP (and, 0x21, INST_REG_MEM16_32_64,   INST_REG16_32_64,       INST_FLD_REG)
_ASM_OP (and, 0x22, INST_REG8,              INST_REG_MEM8, 			INST_FLD_REG)
_ASM_OP (and, 0x23, INST_REG16_32_64,       INST_REG_MEM16_32_64,   INST_FLD_REG)
_ASM_OP2(and, 0x24, INST_REG_AL,			INST_IMM8)
_ASM_OP2(and, 0x25, INST_REG_RAX,			INST_IMM16_32)
/* 0x26 - NULL prefix */
/* 0x27 - invalid */
_ASM_OP (sub, 0x28, INST_REG_MEM8,			INST_REG8,				INST_FLD_REG)
_ASM_OP (sub, 0x29, INST_REG_MEM16_32_64,   INST_REG16_32_64,       INST_FLD_REG)
_ASM_OP (sub, 0x2A, INST_REG8,              INST_REG_MEM8, 			INST_FLD_REG)
_ASM_OP (sub, 0x2B, INST_REG16_32_64,       INST_REG_MEM16_32_64,   INST_FLD_REG)
_ASM_OP2(sub, 0x2C, INST_REG_AL,			INST_IMM8)
_ASM_OP2(sub, 0x2D, INST_REG_RAX,			INST_IMM16_32)
/* 0x2E - NULL prefix */
/* 0x2F - invalid */
_ASM_OP (xor, 0x30, INST_REG_MEM8,			INST_REG8,				INST_FLD_REG)
_ASM_OP (xor, 0x31, INST_REG_MEM16_32_64,   INST_REG16_32_64,       INST_FLD_REG)
_ASM_OP (xor, 0x32, INST_REG8,              INST_REG_MEM8, 			INST_FLD_REG)
_ASM_OP (xor, 0x33, INST_REG16_32_64,       INST_REG_MEM16_32_64,   INST_FLD_REG)
_ASM_OP2(xor, 0x34, INST_REG_AL,			INST_IMM8)
_ASM_OP2(xor, 0x35, INST_REG_RAX,			INST_IMM16_32)
/* 0x36 - NULL prefix */
/* 0x37 - invalid */
_ASM_OP (cmp, 0x38, INST_REG_MEM8,			INST_REG8,				INST_FLD_REG)
_ASM_OP (cmp, 0x39, INST_REG_MEM16_32_64,   INST_REG16_32_64,       INST_FLD_REG)
_ASM_OP (cmp, 0x3A, INST_REG8,              INST_REG_MEM8, 			INST_FLD_REG)
_ASM_OP (cmp, 0x3B, INST_REG16_32_64,       INST_REG_MEM16_32_64,   INST_FLD_REG)
_ASM_OP2(cmp, 0x3C, INST_REG_AL,			INST_IMM8)
_ASM_OP2(cmp, 0x3D, INST_REG_RAX,			INST_IMM16_32)
/* 0x3E - NULL prefix */
/* 0x3F - invalid */
/* 0x4? - extension 64-bit prefix */
_ASM_OP (push, 0x50, INST_REG16_64, 		INST_NONE, INST_SECODE_OP)
_ASM_OP (pop,  0x58, INST_REG16_64,			INST_NONE, INST_SECODE_OP)
/* 0x60 - invalid */
/* 0x61 - invalid */
/* 0x61 - invalid */
/* 0x64 - FS override prefix */
/* 0x65 - GS override prefix */
/* 0x66 - operand override prefix */
/* 0x67 - address override prefix */
_ASM_OP1(push, 0x68, INST_IMM16_32)
_ASM_OP (mul,  0x69, INST_REG16_32_64,		INST_IMM16_32,	INST_FLD_REG)
_ASM_OP1(push, 0x6A, INST_IMM8)
_ASM_OP (mul,  0x6B, INST_REG16_32_64,		INST_IMM8,		INST_FLD_REG)

_ASM_OP1(jo,   0x70, INST_IMM8)
_ASM_OP1(jno   0x71, INST_IMM8)
_ASM_OP1(jc,   0x72, INST_IMM8)
_ASM_OP1(jnc,  0x73, INST_IMM8)
_ASM_OP1(jz,   0x74, INST_IMM8)
_ASM_OP1(jnz,  0x75, INST_IMM8)
_ASM_OP1(jeq,  0x76, INST_IMM8)
_ASM_OP1(neq,  0x77, INST_IMM8)
_ASM_OP1(js,   0x78, INST_IMM8)
_ASM_OP1(jns,  0x79, INST_IMM8)
_ASM_OP1(jp,   0x7A, INST_IMM8)
_ASM_OP1(jnp,  0x7B, INST_IMM8)
_ASM_OP1(jl,   0x7C, INST_IMM8)
_ASM_OP1(jge,  0x7D, INST_IMM8)
_ASM_OP1(jle,  0x7E, INST_IMM8)
_ASM_OP1(jg,   0x7F, INST_IMM8)
_ASM_OP (add,  0x80, INST_REG_MEM8,		 	INST_IMM8,		INST_OPCODE_EXT | 0x00)
_ASM_OP (or,   0x80, INST_REG_MEM8,			INST_IMM8,		INST_OPCODE_EXT | 0x01)
_ASM_OP (adc,  0x80, INST_REG_MEM8,		 	INST_IMM8,		INST_OPCODE_EXT | 0x02)
_ASM_OP (sbb,  0x80, INST_REG_MEM8,		 	INST_IMM8,		INST_OPCODE_EXT | 0x03)
_ASM_OP (and,  0x80, INST_REG_MEM8,		 	INST_IMM8,		INST_OPCODE_EXT | 0x04)
_ASM_OP (sub,  0x80, INST_REG_MEM8,		 	INST_IMM8,		INST_OPCODE_EXT | 0x05)
_ASM_OP (xor,  0x80, INST_REG_MEM8,		 	INST_IMM8,		INST_OPCODE_EXT | 0x06)
_ASM_OP (cmp,  0x80, INST_REG_MEM8,		 	INST_IMM8,		INST_OPCODE_EXT | 0x07)
_ASM_OP (add,  0x81, INST_REG_MEM16_32_64,	INST_IMM16_32,	INST_OPCODE_EXT | 0x00)
_ASM_OP (or,   0x81, INST_REG_MEM16_32_64,	INST_IMM16_32,	INST_OPCODE_EXT | 0x01)
_ASM_OP (adc,  0x81, INST_REG_MEM16_32_64,	INST_IMM16_32,	INST_OPCODE_EXT | 0x02)
_ASM_OP (sbb,  0x81, INST_REG_MEM16_32_64,	INST_IMM16_32,	INST_OPCODE_EXT | 0x03)
_ASM_OP (and,  0x81, INST_REG_MEM16_32_64,	INST_IMM16_32,	INST_OPCODE_EXT | 0x04)
_ASM_OP (sub,  0x81, INST_REG_MEM16_32_64,	INST_IMM16_32,	INST_OPCODE_EXT | 0x05)
_ASM_OP (xor,  0x81, INST_REG_MEM16_32_64,	INST_IMM16_32,	INST_OPCODE_EXT | 0x06)
_ASM_OP (cmp,  0x81, INST_REG_MEM16_32_64,	INST_IMM16_32,	INST_OPCODE_EXT | 0x07)
/* 0x82 - invalid */
_ASM_OP (add,  0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x00)
_ASM_OP (or,   0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x01)
_ASM_OP (adc,  0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x02)
_ASM_OP (sbb,  0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x03)
_ASM_OP (and,  0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x04)
_ASM_OP (sub,  0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x05)
_ASM_OP (xor,  0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x06)
_ASM_OP (cmp,  0x83, INST_REG_MEM16_32_64,	INST_IMM8,		INST_OPCODE_EXT | 0x07)
_ASM_OP (test, 0x84, INST_REG_MEM8,			INST_REG8,				INST_FLD_REG)
_ASM_OP (test, 0x85, INST_REG_MEM16_32_64,	INST_REG16_32_64,		INST_FLD_REG)
_ASM_OP (xchg, 0x86, INST_REG8,				INST_REG_MEM8,			INST_FLD_REG)
_ASM_OP (xchg, 0x87, INST_REG16_32_64,		INST_REG_MEM16_32_64,	INST_FLD_REG)
_ASM_OP (mov,  0x88, INST_REG_MEM8,			INST_REG8,				INST_FLD_REG)
_ASM_OP (mov,  0x89, INST_REG_MEM16_32_64,	INST_REG16_32_64,		INST_FLD_REG)
_ASM_OP (mov,  0x8A, INST_REG8,				INST_REG_MEM8,			INST_FLD_REG)
_ASM_OP (mov,  0x8B, INST_REG16_32_64,		INST_REG_MEM16_32_64,	INST_FLD_REG)
_ASM_OP (mov,  0x8C, INST_MEM16,			INST_REG_SEGMENT,		INST_FLD_REG)
_ASM_OP (mov,  0x8C, INST_REG16_32_64,		INST_REG_SEGMENT,		INST_FLD_REG)
_ASM_OP (lea,  0x8D, INST_REG16_32_64,		INST_MEMALL,			INST_FLD_REG)
_ASM_OP (mov,  0x8E, INST_REG_SEGMENT,		INST_REG_MEM16,			INST_FLD_REG)
_ASM_OP (pop,  0x8F, INST_REG_MEM16_32,		INST_NONE,		INST_OPCODE_EXT | 0x0)
_ASM_OP0(nop,  0x90)
_ASM_OP (xchg, 0x90, INST_REG16_32_64,		INST_REG_RAX,	INST_SECODE_OP)
_ASM_OP (cbw,  0x98, INST_NONE,	INST_NONE,	INST_PF_16BIT)
_ASM_OP0(cwd,  0x98)
_ASM_OP (cdq,  0x98, INST_NONE,	INST_NONE,	INST_PF_EXT)
_ASM_OP (cqd,  0x99, INST_NONE, INST_NONE,	INST_PF_16BIT)
_ASM_OP0(cdq,  0x99)
_ASM_OP (cqo,  0x99, INST_NONE, INST_NONE,	INST_PF_EXT)
/* 0x9A - invalid */
_ASM_OP0(wait,  0x9B)
_ASM_OP0(pushf, 0x9C)
_ASM_OP0(popf,  0x9D)
_ASM_OP0(sahf,  0x9E)
_ASM_OP0(lasf,  0x9F)


_ASM_OP1(lodsb, 0xAC, INST_REG_AL)
_ASM_OP1(lodss, 0xAD, INST_REG_RAX | INST_REG_EAX | INST_REG_AX)

_ASM_OP (mov,  0xB0, INST_REG8,				INST_IMM8,			INST_SECODE_OP)
_ASM_OP (mov,  0xB8, INST_REG16_32_64,		INST_IMM16_32_64,	INST_SECODE_OP)
_ASM_OP (rol,  0xC0, INST_REG_MEM8,			INST_IMM8,			INST_OPCODE_EXT | 0x0)
_ASM_OP (ror,  0xC0, INST_REG_MEM8,			INST_IMM8,			INST_OPCODE_EXT | 0x1)
_ASM_OP (rcl,  0xC0, INST_REG_MEM8,			INST_IMM8,			INST_OPCODE_EXT | 0x2)
_ASM_OP (rcr,  0xC0, INST_REG_MEM8,			INST_IMM8,			INST_OPCODE_EXT | 0x3)
_ASM_OP (shl,  0xC0, INST_REG_MEM8,			INST_IMM8,			INST_OPCODE_EXT | 0x4)
_ASM_OP (shr,  0xC0, INST_REG_MEM8,			INST_IMM8,			INST_OPCODE_EXT | 0x5)
_ASM_OP (sar,  0xC0, INST_REG_MEM8,			INST_IMM8,			INST_OPCODE_EXT | 0x7)
_ASM_OP (rol,  0xC1, INST_REG_MEM16_32_64,	INST_IMM8,			INST_OPCODE_EXT | 0x0)
_ASM_OP (ror,  0xC1, INST_REG_MEM16_32_64,	INST_IMM8,			INST_OPCODE_EXT | 0x1)
_ASM_OP (rcl,  0xC1, INST_REG_MEM16_32_64,	INST_IMM8,			INST_OPCODE_EXT | 0x2)
_ASM_OP (rcr,  0xC1, INST_REG_MEM16_32_64,	INST_IMM8,			INST_OPCODE_EXT | 0x3)
_ASM_OP (shl,  0xC1, INST_REG_MEM16_32_64,	INST_IMM8,			INST_OPCODE_EXT | 0x4)
_ASM_OP (shr,  0xC1, INST_REG_MEM16_32_64,	INST_IMM8,			INST_OPCODE_EXT | 0x5)
_ASM_OP (sar,  0xC1, INST_REG_MEM16_32_64,	INST_IMM8,			INST_OPCODE_EXT | 0x7)
_ASM_OP1(ret,  0xC2, INST_IMM16)
_ASM_OP0(ret,  0xC3)
/* 0xC4 - invalid */
/* 0xC5 - invalid */
_ASM_OP (mov,  0xC6, INST_REG8_MEM8,		INST_IMM8,		INST_OPCODE_EXT | 0x0)
_ASM_OP (mov,  0xC7, INST_REG_MEM16_32_64,	INST_IMM16_32,	INST_OPCODE_EXT | 0x0)
_ASM_OP1(retf, 0xCA, INST_IMM16)
_ASM_OP0(retf, 0xCB)
_ASM_OP1(int,  0xCC, INST_CONST | 0x03)
_ASM_OP1(int,  0xCD, INST_IMM8)
_ASM_OP0(iret, 0xCF)

_ASM_OP0(iret,  0xCF)
_ASM_OP (iretq, 0xCF, INST_NONE,	INST_NONE,	INST_PF_EXT)

_ASM_OP (rol,  0xD0, INST_REG8_MEM8,		INST_CONST | 1,	INST_OPCODE_EXT | 0x0)
_ASM_OP (ror,  0xD0, INST_REG8_MEM8,		INST_CONST | 1,	INST_OPCODE_EXT | 0x1)
_ASM_OP (rcl,  0xD0, INST_REG8_MEM8,		INST_CONST | 1,	INST_OPCODE_EXT | 0x2)
_ASM_OP (rcr,  0xD0, INST_REG8_MEM8,		INST_CONST | 1,	INST_OPCODE_EXT | 0x3)
_ASM_OP (shl,  0xD0, INST_REG8_MEM8,		INST_CONST | 1,	INST_OPCODE_EXT | 0x4)
_ASM_OP (shr,  0xD0, INST_REG8_MEM8,		INST_CONST | 1,	INST_OPCODE_EXT | 0x5)
_ASM_OP (sar,  0xD0, INST_REG8_MEM8,		INST_CONST | 1,	INST_OPCODE_EXT | 0x7)
_ASM_OP (rol,  0xD1, INST_REG8_MEM8,		INST_CONST | 1,	INST_OPCODE_EXT | 0x0)
_ASM_OP (ror,  0xD1, INST_REG8_MEM8,		INST_CONST | 1,	INST_OPCODE_EXT | 0x1)
_ASM_OP (rcl,  0xD1, INST_REG8_MEM8,		INST_CONST | 1,	INST_OPCODE_EXT | 0x1)
_ASM_OP (rcr,  0xD1, INST_REG8_MEM8,		INST_CONST | 1,	INST_OPCODE_EXT | 0x3)
_ASM_OP (shl,  0xD1, INST_REG8_MEM8,		INST_CONST | 1,	INST_OPCODE_EXT | 0x4)
_ASM_OP (shr,  0xD1, INST_REG8_MEM8,		INST_CONST | 1,	INST_OPCODE_EXT | 0x5)
_ASM_OP (sar,  0xD1, INST_REG8_MEM8,		INST_CONST | 1,	INST_OPCODE_EXT | 0x7)
_ASM_OP (rol,  0xD2, INST_REG8_MEM8,		INST_REG_CL,	INST_OPCODE_EXT | 0x0)
_ASM_OP (ror,  0xD2, INST_REG8_MEM8,		INST_REG_CL,	INST_OPCODE_EXT | 0x1)
_ASM_OP (rcl,  0xD2, INST_REG8_MEM8,		INST_REG_CL,	INST_OPCODE_EXT | 0x2)
_ASM_OP (rcr,  0xD2, INST_REG8_MEM8,		INST_REG_CL,	INST_OPCODE_EXT | 0x3)
_ASM_OP (shl,  0xD2, INST_REG8_MEM8,		INST_REG_CL,	INST_OPCODE_EXT | 0x4)
_ASM_OP (shr,  0xD2, INST_REG8_MEM8,		INST_REG_CL,	INST_OPCODE_EXT | 0x5)
_ASM_OP (sar,  0xD2, INST_REG8_MEM8,		INST_REG_CL,	INST_OPCODE_EXT | 0x7)
_ASM_OP (rol,  0xD3, INST_REG_MEM16_32_64,	INST_REG_CL,	INST_OPCODE_EXT | 0x0)
_ASM_OP (ror,  0xD3, INST_REG_MEM16_32_64,	INST_REG_CL,	INST_OPCODE_EXT | 0x1)
_ASM_OP (rcl,  0xD3, INST_REG_MEM16_32_64,	INST_REG_CL,	INST_OPCODE_EXT | 0x2)
_ASM_OP (rcr,  0xD3, INST_REG_MEM16_32_64,	INST_REG_CL,	INST_OPCODE_EXT | 0x3)
_ASM_OP (shl,  0xD3, INST_REG_MEM16_32_64,	INST_REG_CL,	INST_OPCODE_EXT | 0x4)
_ASM_OP (shr,  0xD3, INST_REG_MEM16_32_64,	INST_REG_CL,	INST_OPCODE_EXT | 0x5)
_ASM_OP (sar,  0xD3, INST_REG_MEM16_32_64,	INST_REG_CL,	INST_OPCODE_EXT | 0x7)
/* 0xD4 - invalid */
/* 0xD5 - invalid */
/* 0xD6 - invalid */
_ASM_OP1(loopnz, 0xE0, INST_IMM8)
_ASM_OP1(loopz,  0xE1, INST_IMM8)
_ASM_OP1(loop,   0xE2, INST_IMM8)
_ASM_OP1(jcxz,   0xE3, INST_IMM8)
_ASM_OP2(in,     0xE4, INST_REG_AL,		INST_IMM8)
_ASM_OP2(in,     0xE5, INST_REG_EAX,	INST_IMM8)
_ASM_OP2(out,    0xE6, INST_IMM8,		INST_REG_AL)
_ASM_OP2(out,    0xE7, INST_IMM8,		INST_REG_EAX)
_ASM_OP1(call,   0xE8, INST_IMM16_32)
_ASM_OP1(jmp,    0xE9, INST_IMM16_32)
/* 0xEA - invalid */
_ASM_OP1(jmp,    0xEB, INST_IMM8)
_ASM_OP2(in,     0xEC, INST_REG_AL,		INST_REG_DX)
_ASM_OP2(in,     0xED, INST_REG_EAX,	INST_REG_DX)
_ASM_OP2(out,    0xEE, INST_REG_DX,		INST_REG_AL)
_ASM_OP2(out,    0xEF, INST_REG_DX,		INST_REG_EAX)
/* 0xF0 - lock-prefix */

_ASM_OP0(hlt,  0xF4)	/* Halt */
_ASM_OP0(cmc,  0xF5)	/* Complement Carry Bit */
_ASM_OP (test, 0xF6, INST_REG_MEM8,			INST_IMM16_32,	INST_OPCODE_EXT | 0x0)
_ASM_OP (not,  0xF6, INST_REG_MEM8,			INST_NONE,		INST_OPCODE_EXT | 0x2)
_ASM_OP (neg,  0xF6, INST_REG_MEM8,			INST_NONE,		INST_OPCODE_EXT | 0x3)
_ASM_OP (div,  0xF6, INST_REG_MEM8,			INST_NONE,		INST_OPCODE_EXT | 0x6)
_ASM_OP (idiv, 0xF6, INST_REG_MEM8,			INST_NONE,		INST_OPCODE_EXT | 0x7)
_ASM_OP (test, 0xF7, INST_REG_MEM16_32_64,	INST_IMM16_32,	INST_OPCODE_EXT | 0x0)
_ASM_OP (not,  0xF7, INST_REG_MEM16_32_64,	INST_NONE,		INST_OPCODE_EXT | 0x2)
_ASM_OP (neg,  0xF7, INST_REG_MEM16_32_64,	INST_NONE,		INST_OPCODE_EXT | 0x3)
_ASM_OP (div,  0xF7, INST_REG_MEM16_32_64,	INST_NONE,		INST_OPCODE_EXT | 0x6)
_ASM_OP (idiv, 0xF7, INST_REG_MEM16_32_64,	INST_NONE,		INST_OPCODE_EXT | 0x7)
_ASM_OP0(clc,  0xF8)	/* Clear Carry     Bit */
_ASM_OP0(stc,  0xF9)	/* Set   Carry     Bit */
_ASM_OP0(cli,  0xFA)	/* Clear Interrupt Bit*/
_ASM_OP0(sti,  0xFB)	/* Set   Interrupt Bit */
_ASM_OP0(cld,  0xFC)	/* Clear Direction Bit */
_ASM_OP0(std,  0xFD)	/* Set   Direction Bit */
_ASM_OP (inc,  0xFE, INST_REG8_MEM8,		INST_NONE,		INST_OPCODE_EXT | 0x0)
_ASM_OP (dec,  0xFE, INST_REG8_MEM8,		INST_NONE,		INST_OPCODE_EXT | 0x1)
_ASM_OP (inc,  0xFF, INST_REG_MEM16_32, 	INST_NONE,		INST_OPCODE_EXT | 0x0)
_ASM_OP (dec,  0xFF, INST_REG_MEM16_32, 	INST_NONE,		INST_OPCODE_EXT | 0x1)
_ASM_OP (call, 0xFF, INST_REG_MEM64,		INST_NONE,		INST_OPCODE_EXT | 0x2)
_ASM_OP (jmp,  0xFF, INST_REG_MEM64,		INST_NONE,		INST_OPCODE_EXT | 0x4)
_ASM_OP (push, 0xFF, INST_REG_MEM16_64,		INST_NONE,		INST_OPCODE_EXT | 0x6)

/* 0x0F two-byte prefix */
_ASM_OP_PF_TB(syscall, 0x05, INST_NONE,			INST_NONE,	INST_NONE)
_ASM_OP_PF_TB(je,      0x84, INST_IMM16_32,		INST_NONE,  INST_NONE)
_ASM_OP_PF_TB(jz,      0x84, INST_IMM16_32,		INST_NONE,  INST_NONE)
_ASM_OP_PF_TB(jne,     0x85, INST_IMM16_32,		INST_NONE,  INST_NONE)
_ASM_OP_PF_TB(jbe,     0x86, INST_IMM16_32,		INST_NONE,  INST_NONE)
_ASM_OP_PF_TB(jnbe,    0x87, INST_IMM16_32,		INST_NONE,  INST_NONE)
_ASM_OP_PF_TB(js,      0x88, INST_IMM16_32,		INST_NONE,  INST_NONE)
_ASM_OP_PF_TB(jns,     0x89, INST_IMM16_32,		INST_NONE,  INST_NONE)
_ASM_OP_PF_TB(jpe,     0x8A, INST_IMM16_32,		INST_NONE,  INST_NONE)
_ASM_OP_PF_TB(jpo,     0x8B, INST_IMM16_32,		INST_NONE,  INST_NONE)
_ASM_OP_PF_TB(jl,      0x8C, INST_IMM16_32,		INST_NONE,  INST_NONE)
_ASM_OP_PF_TB(jge,     0x8D, INST_IMM16_32,		INST_NONE,  INST_NONE)
_ASM_OP_PF_TB(jle,     0x8E, INST_IMM16_32,		INST_NONE,  INST_NONE)
_ASM_OP_PF_TB(jg,      0x8F, INST_IMM16_32,		INST_NONE,  INST_NONE)
_ASM_OP_PF_TB(seto,    0x90, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(setno,   0x91, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(setc,    0x92, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(setnc,   0x93, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(setz,    0x94, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(setnz,   0x95, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(setcz,   0x96, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(setncz,  0x97, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(sets,    0x98, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(setns,   0x99, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(setp,    0x9A, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(setnp,   0x9B, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(setl,    0x9C, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(setnl,   0x9D, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(setng,   0x9E, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(setg,    0x9F, INST_MEM_REG8,		INST_NONE,  INST_OPCODE_EXT)
_ASM_OP_PF_TB(mul,     0xAF, INST_REG16_32_64,	INST_REG_MEM16_32_64,	INST_FLD_REG)

#endif /* __ZASM_X86_64_INSTRUCT_H__ */

