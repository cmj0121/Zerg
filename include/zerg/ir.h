/* Copyright (C) 2016-2016 cmj. All right reserved. */

#ifndef __ZERG_IR_H__
#	define __ZERG_IR_H__


#include "zasm.h"

#define __IR_REFERENCE__	"&"
#define __IR_VERSION__		"0.1 (" __DATE__ " " __TIME__ ")"
#define __IR_REG_FMT__		".reg.%02d"
#define __IR_LABEL_FMT__	".zerg.label.%d"
#define __IR_SYSCALL_REG__	".reg.sys"
#define __IR_LOCAL_VAR__	".local.var"
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
	IROP_UNKNOWN,	/* unknown IR op */

	/* data-related */
	IROP_XCHG,
	IROP_STORE,
	IROP_LOAD,

	/* arithmetic */
	IROP_ADD,
	IROP_SUB,
	IROP_MUL,
	IROP_DIV,
	IROP_REM,
	IROP_INC,
	IROP_DEC,
	IROP_SHL,
	IROP_SHR,

	/* logical */
	IROP_AND,
	IROP_OR,
	IROP_XOR,
	IROP_NOT,
	IROP_NEG,
	IROP_EQ,
	IROP_LS,
	IROP_LE,
	IROP_GE,
	IROP_GT,

	/* control flow */
	IROP_JMP,
	IROP_JMPIF,

	/* subroutine */
	IROP_CALL,
	IROP_RET,
	IROP_PARAM,
	IROP_PROLOGUE,
	IROP_EPILOGUE,

	/* extra */
	IROP_INTERRUPT,
	IROP_NOP,
	IROP_LABEL,
	IROP_ASM,
} IROP;

class IR : public Binary {
	public:
		IR(std::string dst, ZergArgs *args);
		~IR(void);

		/* compile the IR from source code */
		void compile(std::string src);
		/* emit from IR to machine code */
		void emit(STRING op, STRING dst, STRING src, STRING idx, STRING size);
		IROP opcode(std::string src);

		virtual std::string regalloc(std::string src, std::string size="") = 0;
		virtual void regsave(std::string src) = 0;
		virtual void resetreg(void) = 0;
		virtual std::string tmpreg(void) = 0;
	protected:
		std::string randstr(unsigned int size=24, std::string prefix=".");
		std::vector<std::string> _repeate_label_, _stack_;
	private:
		int _param_nr_, _lineno_;
		ZergArgs *_args_;
};

#endif /* __ZERG_IR_H__ */
