/* Copyright (C) 2016-2016 cmj. All right reserved. */

#ifndef __ZERG_IR_H__
#	define __ZERG_IR_H__

#include <vector>
#include <fstream>
#include <string>

#include "zasm.h"

#define MAX_ZASMP_PARAM	10
#define __IR_VERSION__		"0.1 (" __DATE__ " " __TIME__ ")"
#define __IR_REG_FMT__		".reg.%02d"
#define __IR_LABEL_FMT__	".zerg.label.%d"
#define __IR_SYSCALL_REG__	".reg.sys"
#define __IR_LOCAL_VAR__	".local.var"
#define __IR_DUMMY__		".dummy"

class IRToken {
	public:
		IRToken(std::string src);

		size_t length(void) const;

		std::string op(void)   const;
		std::string dst(void)  const;
		std::string src(void)  const;
		std::string idx(void)  const;
		std::string size(void) const;

		friend std::ostream& operator <<(std::ostream &stream, const IRToken &src);
	private:
		std::vector<std::string> _src_;
};

class IR : public Binary {
	public:
		IR(std::string dst, ZergArgs *args);
		~IR(void);

		void emit(IRToken *token);
		void emit(std::string op, std::string dst, std::string src, std::string idx, std::string size);
		void compile(std::string src);

		virtual std::string regalloc(std::string src, std::string size="") = 0;
		virtual void regsave(std::string src) = 0;
		virtual std::string tmpreg(void) = 0;
	protected:
		bool _only_ir_;
		std::string randstr(int size=24, std::string prefix="__", std::string suffix="__");
		std::vector<std::string> _repeate_label_, _stack_;
	private:
		int _param_nr_;
		bool _symb_;
		off_t _entry_;
		std::string _param_[MAX_ZASMP_PARAM];
};

#endif /* __ZERG_IR_H__ */
