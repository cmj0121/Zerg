/* Copyright (C) 2014-2016 cmj. All right reserved. */
#ifndef __ZASM_X86_64_INST_H__
#  define __ZASM_X86_64_INST_H__

#define OPERAND_SIZE_OFFSET		8
#define OPERAND_SIZE(ctx)	 	((ctx) >> OPERAND_SIZE_OFFSET)

#define MAX_CMD_LEN	8
typedef struct _x86_64_inst_tag_ {
	char cmd[MAX_CMD_LEN];
	unsigned char opcode;
	unsigned int op1, op2;
	unsigned int flags;
} X86_64_INST;

enum _x86_64_inst_type_ {
	INST_NONE		= 0x0000,

	INST_IMM		= 0x0002,
	INST_MEM		= 0x0004,
	INST_REG		= 0x0008,

	INST_SIZE8		= 0x0010,
	INST_SIZE16		= 0x0020,
	INST_SIZE32		= 0x0040,
	INST_SIZE64		= 0x0080,
	INST_SIZE128	= 0x0100,	/* XMM - 128-bit registers */
	INST_SIZE256	= 0x0200,	/* YMM - 256-bit registers */
	INST_SIZE512	= 0x0400,	/* ZMM - 512-bit registers */
	INST_SIZE_ALL	= INST_SIZE8 | INST_SIZE16 | INST_SIZE32 | INST_SIZE64,

	INST_SIZE16_32		= INST_SIZE16 | INST_SIZE32,
	INST_SIZE16_32_64	= INST_SIZE16 | INST_SIZE32 | INST_SIZE64,

	INST_IMM8			= INST_IMM | INST_SIZE8,
	INST_IMM16			= INST_IMM | INST_SIZE16,
	INST_IMM64			= INST_IMM | INST_SIZE64,
	INST_IMM16_32		= INST_IMM | INST_SIZE16_32,
	INST_IMM16_32_64	= INST_IMM | INST_SIZE16_32_64,

	INST_MEM8			= INST_MEM | INST_SIZE8,
	INST_MEM16			= INST_MEM | INST_SIZE16,
	INST_MEM32			= INST_MEM | INST_SIZE32,
	INST_MEM64			= INST_MEM | INST_SIZE64,
	INST_MEM16_32		= INST_MEM | INST_SIZE16_32,
	INST_MEM16_32_64	= INST_MEM | INST_SIZE16_32_64,

	INST_REG8			= INST_REG | INST_SIZE8,
	INST_REG16			= INST_REG | INST_SIZE16,
	INST_REG64			= INST_REG | INST_SIZE64,
	INST_REG16_32		= INST_REG | INST_SIZE16_32,
	INST_REG16_32_64	= INST_REG | INST_SIZE16_32_64,
	INST_XMM			= INST_REG | INST_SIZE128,
	INST_YMM			= INST_REG | INST_SIZE256,
	INST_ZMM			= INST_REG | INST_SIZE512,

	INST_REG_MEM16_32_64	= INST_REG16_32_64 | INST_MEM16_32_64,

	INST_TWO_BYTE		= 0x01000,	/* Two byte opcode */
	INST_SECONDARY		= 0x02000,	/* Appended value to the opcode */
	INST_OPCODE_EXT		= 0x04000,	/* Register/Opcode Field */
	INST_REG_OPERANDS	= 0x08000,	/* Register operand field */

	INST_OPCODE_MULTI	= 0x10000,
	INST_REG_SWAP		= 0x20000,
	INST_EXTENSION_64	= 0x40000,
	INST_SINGLE_FP		= 0x80000,
	INST_DOUBLE_FP		= 0x100000,

#ifdef __x86_64__		/* Special src */
	INST_REG_RAX		= 0x1000000,
	INST_REG_AH			= 0x1000001,
	INST_REG_AL			= 0x1000002,
	INST_REG_RCX		= 0x2000000,
	INST_REG_CL			= 0x2000001,
	INST_REG_SPECIFY	= 0xF000000,
#endif /* __x86_64__ */
	INST_ERR			= 0xFFFFFFFF,
};

static X86_64_INST InstructionSets[] = {
	#define INST_ASM_OP(cmd, op, x, y, flag) 			{ #cmd, op, x, y, flag },
	#define INST_ASM_OP_TWOBYTE(cmd, op, x, y, flag)	INST_ASM_OP( cmd, op, x, y, flag | INST_TWO_BYTE)
	#define INST_ASM_OP0(cmd, op) 						INST_ASM_OP( cmd, op, INST_NONE, INST_NONE, INST_NONE)
	#define INST_ASM_OP1(cmd, op, x)					INST_ASM_OP( cmd, op, x,         INST_NONE, INST_NONE)
	#define INST_ASM_OP2(cmd, op, x, y)					INST_ASM_OP( cmd, op, x,         y,		    INST_NONE)

	#  include "x86_64_opcodes.h"
	#undef INST_ASM_OP
	#undef INST_ASM_OP_TWOBYTE
	#undef INST_ASM_OP0
	#undef INST_ASM_OP1
};

#define REG_GENERAL_64		"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi"
#define REG_GENERAL_32		"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"
#define REG_GENERAL_16		 "ax",  "cx",  "dx",  "bx",  "sp",  "bp",  "si",  "di"
#define REG_GENERAL_8		 "al",  "cl",  "dl",  "bl", "spl", "bpl", "sil", "dil", \
							 "ah",  "ch",  "dh",  "bh"
#define REG_GENERAL			REG_GENERAL_64, REG_GENERAL_32, REG_GENERAL_16, REG_GENERAL_8

#define REG_EXTENSION_64	"r8",  "r9",  "r10",  "r11",  "r12",  "r13",  "r14",  "r15"
#define REG_EXTENSION_32	"r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"
#define REG_EXTENSION_16	"r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w"
#define REG_EXTENSION_8		"r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"
#define REG_EXTENSION		REG_EXTENSION_64, REG_EXTENSION_32, REG_EXTENSION_16, REG_EXTENSION_8

#define REGISTERS			REG_EXTENSION, REG_GENERAL
#define USED_REGISTERS		"rcx", "rbx", "r8", "r9", "r10", "r11", 

#define SYSCALL_REG			"rax"
#endif /* __ZASM_X86_64_INST_H__ */
