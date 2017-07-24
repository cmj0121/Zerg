/* Copyright (C) 2016-2016 cmj. All right reserved. */

#ifndef __ZERG_IR_H__
#	define __ZERG_IR_H__


#include "zasm.h"

#define __IR_VERSION__		"1.1"

#define __IR_REFERENCE__	"&"
#define __IR_REG_FMT__		".reg.%02d"
#define __IR_LABEL_FMT__	".zerg.label.%d"
#define __IR_SYSCALL_REG__	".reg.sys"
#define __IR_FUNC_STACK__	".function.stack"
#define __IR_DUMMY__		".dummy"

/* Control-Flow */
#define __IR_LABEL_IF__		".if.%04d"
#define __IR_LABEL_WHILE__	".while.%04d"
#define __IR_LABEL_END__	"%s.end"
#define __IR_LABEL_FALSE__	"%s.false"

/* Category of class property */
#define __IR_CLS_NAME__		".name.%s"
#define __IR_CLS_REG__		".reg.%s.%s"
#define __IR_CLS_FUNC__		".fun.%s.%s"
#define __IR_CLS_OBJ__		".obj.%s.%s"

#include <vector>
#include <string>

typedef enum _tag_ir_ {
	IR_UNKNOWN = 0,	/* unknown IR op */

	/* data-related */
	IR_MEMORY_STORE,
	IR_MEMORY_LOAD,
	IR_MEMORY_XCHG,
	IR_MEMORY_PUSH,
	IR_MEMORY_POP,

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
	IR_CONDITION_JMPIF,
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

typedef enum _tag_ir_type_ {
	IR_TOKEN_UNKNOWN	= 0,
	IR_TOKEN_INT,
	IR_TOKEN_REGISTER,
	IR_TOKEN_VAR,
	IR_TOKEN_REF,
	IR_TOKEN_STRING,
} IRType;

const std::vector<std::pair<std::string, IROP>> IROP_map = {
	/* memory access*/
	{"STORE"	, IR_MEMORY_STORE},
	{"LOAD"		, IR_MEMORY_LOAD},
	{"XCHG"		, IR_MEMORY_XCHG},
	{"PUSH"		, IR_MEMORY_PUSH},
	{"POP"		, IR_MEMORY_POP},
	/* arithmetic operation */
	{"ADD"		, IR_ARITHMETIC_ADD},
	{"SUB"		, IR_ARITHMETIC_SUB},
	{"MUL"		, IR_ARITHMETIC_MUL},
	{"DIV"		, IR_ARITHMETIC_DIV},
	{"MOD"		, IR_ARITHMETIC_MOD},
	{"SHR"		, IR_ARITHMETIC_SHR},
	{"SHL"		, IR_ARITHMETIC_SHL},
	{"INC"		, IR_ARITHMETIC_INC},
	{"DEC"		, IR_ARITHMETIC_DEC},
	/* logical operation */
	{"AND"		, IR_LOGICAL_AND},
	{"OR"		, IR_LOGICAL_OR},
	{"XOR"		, IR_LOGICAL_XOR},
	{"NOT"		, IR_LOGICAL_NOT},
	{"NEG"		, IR_LOGICAL_NEG},
	{"EQ"		, IR_LOGICAL_EQ},
	{"LS"		, IR_LOGICAL_LS},
	{"GT"		, IR_LOGICAL_GT},
	/* condition / control flow */
	{"JMP"		, IR_CONDITION_JMP},
	{"JMPIF"	, IR_CONDITION_JMPIF},
	{"CALL"		, IR_CONDITION_CALL},
	{"RET"		, IR_CONDITION_RET},
	/* extra */
	{"NOP"		, IR_NOP},
	{"PROLOGUE"	, IR_PROLOGUE},
	{"EPILOGUE"	, IR_EPILOGUE},
	{"INTERRUPT", IR_INTERRUPT},
	{"LABEL"	, IR_LABEL},
	{"DEFINE"	, IR_DEFINE},
	{"INLINE"	, IR_INLINE_ASM},
};

class IR : public Zasm {
	public:
		IR(std::string dst, Args &args);
		virtual ~IR(void);

		/* compile the IR from source code */
		void compile(std::string src);

		/* emit from IR to machine code */
		void emit(STRING op, STRING dst, STRING src, STRING size, STRING index);
		void emit(IROP op, STRING dst="", STRING src="", STRING size="",STRING idx="");
		IROP opcode(std::string src);

		std::string regalloc(std::string src, std::string size);
		std::string localvar(std::string src, std::string size, std::string idx);
	protected:
		std::string randstr(unsigned int size=24, std::string prefix=".");
		std::vector<std::string> _repeate_label_;
	private:
		int _lineno_;
		size_t _param_nr_;
		Args _args_;

		std::vector<std::string> _alloc_regs_ = { USED_REGISTERS };
		std::map<std::string, std::string> _alloc_regs_map_;
		std::vector<std::string> _local_;

		IRType token(std::string src);
};

#endif /* __ZERG_IR_H__ */
