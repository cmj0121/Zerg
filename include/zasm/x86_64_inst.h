/* Copyright (C) 2014-2016 cmj. All right reserved. */
#ifndef __ZASM_X86_64_INST_H__
#  define __ZASM_X86_64_INST_H__

#define PARAM_SIZE				0x08
#define MAX_CMD_LEN				8

#include "zasm/registers.h"

#define USED_REGISTERS		"rcx", "rbx", "r8", "r9", "r10", "r11", 
#define SYSCALL_PARAM		"rax", "rdi", "rsi", "rdx", "r10", "r8", "r9"
#define SYSCALL_REG			"rax"

#define INST_PF_TWOBYTE		0x00001	<< INST_EXTRA_SHT	/* 0x0F two-byte prefix */
#define INST_PF_EXT			0x00002 << INST_EXTRA_SHT	/* 0x4? operand override prefix */
#define INST_PF_16BIT		0x00004 << INST_EXTRA_SHT	/* 0x66 operand override prefix */
#define INST_PF_WAIT		0x00008 << INST_EXTRA_SHT	/* 0x9B wait prefix */
#define INST_PF_LOCK		0x00010 << INST_EXTRA_SHT	/* 0xF0 lock prefix */
#define INST_FLD_REG		0x00100 << INST_EXTRA_SHT	/* Add register into opcode */
#define INST_SECODE_OP		0x00200 << INST_EXTRA_SHT	/* second opcode */
#define INST_OPCODE_EXT		0x01000 << INST_EXTRA_SHT	/* opcode extension */

#define INST_REG_RAX		(0x10000 << INST_EXTRA_SHT)
#define INST_REG_EAX		INST_REG_RAX | 0x01
#define INST_REG_AX			INST_REG_RAX | 0x02
#define INST_REG_AH			INST_REG_RAX | 0x04
#define INST_REG_AL			INST_REG_RAX | 0x08
#define INST_REG_RCX		(0x20000 << INST_EXTRA_SHT)
#define INST_REG_ECX		INST_REG_RCX | 0x01
#define INST_REG_CX			INST_REG_RCX | 0x02
#define INST_REG_CH			INST_REG_RCX | 0x04
#define INST_REG_CL			INST_REG_RCX | 0x08
#define INST_REG_RDX		(0x30000 << INST_EXTRA_SHT)
#define INST_REG_EDX		INST_REG_RDX | 0x01
#define INST_REG_DX			INST_REG_RDX | 0x02
#define INST_REG_DH			INST_REG_RDX | 0x04
#define INST_REG_DL			INST_REG_RDX | 0x08

#define INST_REG_SEGMENT	(0x80000 << INST_EXTRA_SHT)
#define INST_REG_SEG_ES		INST_REG_SEGMENT | 0x01
#define INST_REG_SEG_CS		INST_REG_SEGMENT | 0x02
#define INST_REG_SEG_SS		INST_REG_SEGMENT | 0x04
#define INST_REG_SEG_DS		INST_REG_SEGMENT | 0x08
#define INST_REG_SEG_FS		INST_REG_SEGMENT | 0x10
#define INST_REG_SEG_GS		INST_REG_SEGMENT | 0x20
#define INST_REG_EFLAGS		(0x90000 << INST_EXTRA_SHT)
#define INST_REG_SPECIFY	(0xF0000 << INST_EXTRA_SHT)

typedef struct _x86_64_inst_tag_ {
	uint8_t cmd[MAX_CMD_LEN];
	uint8_t opcode;
	uint64_t op1, op2, flags;
} X86_64_INST;

#endif /* __ZASM_X86_64_INST_H__ */
