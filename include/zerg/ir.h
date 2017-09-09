/* Copyright (C) 2016-2016 cmj. All right reserved. */

#ifndef __ZERG_IR_H__
#	define __ZERG_IR_H__


#include "zasm.h"

#define __IR_VERSION__		"1.1"

#define __IR_REFERENCE__	"&"
#define __IR_REG_FMT__		".reg.%02d"
#define __IR_REG_TMP__		".reg.temp"
#define __IR_LABEL_FMT__	".zerg.label.%d"
#define __IR_SYSCALL_REG__	".reg.sys"
#define __IR_FUNC_STACK__	".func.param.%d"

/* Control-Flow */
#define __IR_BRANCH__		".branch.%04d"
#define __IR_BRANCH_E__		".end"
#define __IR_BRANCH_END__	".branch.%04d" __IR_BRANCH_E__

/* Category of class property */
#define __IR_CLS_NAME__		".name.%s"
#define __IR_CLS_REG__		".reg.%s.%s"
#define __IR_CLS_FUNC__		".fun.%s.%s"
#define __IR_CLS_OBJ__		".obj.%s.%s"

#include <vector>
#include <map>
#include <string>

typedef enum _tag_ir_opcode_ {
	IR_UNKNOWN = 0,	/* unknown IR op */

	/* data-related */
	IR_MEMORY_STORE,
	IR_MEMORY_LOAD,
	IR_MEMORY_XCHG,
	IR_MEMORY_PUSH,
	IR_MEMORY_POP,
	IR_MEMORY_PARAM,

	/* arithmetic */
	IR_ARITHMETIC_ADD,
	IR_ARITHMETIC_SUB,
	IR_ARITHMETIC_MUL,
	IR_ARITHMETIC_DIV,
	IR_ARITHMETIC_MOD,
	IR_ARITHMETIC_SHL,
	IR_ARITHMETIC_SHR,
	IR_ARITHMETIC_INC,
	IR_ARITHMETIC_DEC,

	/* logical */
	IR_LOGICAL_AND,
	IR_LOGICAL_OR,
	IR_LOGICAL_XOR,
	IR_LOGICAL_NOT,
	IR_LOGICAL_NEG,
	IR_LOGICAL_EQ,
	IR_LOGICAL_LS,
	IR_LOGICAL_GT,

	/* control flow */
	IR_CONDITION_JMP,
	IR_CONDITION_JMPIFN,
	IR_CONDITION_CALL,
	IR_CONDITION_RET,

	/* extra */
	IR_NOP,
	IR_PROLOGUE,
	IR_EPILOGUE,
	IR_INTERRUPT,
	IR_LABEL,
	IR_DEFINE,
	IR_INLINE_ASM,
} IROP;

typedef struct _tag_ir_inst_ {
	IROP opcode;
	std::string dst, src, size, index;
	int lineno;
} IRInstruction;
typedef std::vector<IRInstruction>::iterator IRInstIter;

class IR : public Zasm {
	public:
		IR(std::string dst, Args &args);
		virtual ~IR(void) {};

		/* compile the IR from source code */
		void compile(std::string src);
		void compileL(std::string src);
		void emit_x64(size_t irpos);

		virtual void dump();

		friend IR& operator+= (IR &src, IRInstruction &inst);
	private:
		int _lineno_, _syscall_nr_;
		Args _args_;

		std::vector<IRInstruction> _irs_;
		std::vector<std::string> _alloc_regs_ = { USED_REGISTERS };
		std::map<std::string, std::string> _alloc_regs_map_;
		std::vector<std::string> _locals_, _params_;

		void regalloc(void);
		bool is_register(std::string src);
		std::string regalloc(int irpos, std::string src, std::string size="");
		std::string variable(std::string src);
};

#endif /* __ZERG_IR_H__ */
