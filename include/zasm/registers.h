/* Copyright (C) 2015-2017 cmj. All right reserved. */
#ifndef __ZASM_REGISTER_H__
#  define  __ZASM_REGISTER_H__

#define X86_REG64		"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi"
#define X86_REG32		"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"
#define X86_REG16		"ax",  "cx",  "dx",  "bx",  "sp",  "bp",  "si",  "di"
#define X86_REG8		"al",  "cl",  "dl",  "bl", "spl", "bpl", "sil", "dil"
#define X86_REG_LOWER8	"ah",  "ch",  "dh",  "bh"
#define X86_REGS		X86_REG64, X86_REG32, X86_REG16, X86_REG8, X86_REG_LOWER8

#define X86_EXTREG64	"r8",  "r9",  "r10",  "r11",  "r12",  "r13",  "r14",  "r15"
#define X86_EXTREG32	"r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"
#define X86_EXTREG16	"r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w"
#define X86_EXTREG8		"r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"
#define X86_EXTREGS		X86_EXTREG64, X86_EXTREG32, X86_EXTREG16, X86_EXTREG8

#define SEGREG_ES		"es"	/* destination segment */
#define SEGREG_CS		"cs"	/* code segment - IP   */
#define SEGREG_SS		"ss"	/* stack segment       */
#define SEGREG_DS		"ds"	/* data segment        */
#define SEGREG_FS		"fs"
#define SEGREG_GS		"gs"
#define SEGREGS			SEGREG_ES, SEGREG_CS, SEGREG_SS, SEGREG_DS, SEGREG_FS, SEGREG_GS

#define REGISTERS		X86_REGS, X86_EXTREGS, SEGREGS

#endif /* __ZASM_REGISTER_H__ */

