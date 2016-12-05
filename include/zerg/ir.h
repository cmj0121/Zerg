/* Copyright (C) 2016-2016 cmj. All right reserved. */

#ifndef __ZERG_IR_H__
#	define __ZERG_IR_H__

#include <vector>
#include <fstream>
#include <string>

#include "zasm.h"

#define MAX_ZASMP_PARAM	10
#define __IR_VERSION__		"0.1 (" __DATE__ " " __TIME__ ")"
#define __IR_REG_FMT__		"REG_%02d"
#define __IR_LABEL_FMT__	"__ZERG_LABEL_%d__"
#define __IR_SYSCALL_REG__	"SYSCALL_REG"
#define __IR_LOCAL_VAR__	"LOCAL"

/* Low-Level IR to emit Zasm language
 *
 * This IR is a max-to-4 tuple (OP, DST, SRC, EXTRA) and support 40 pseudo operators
 */
class IRToken {
	public:
		IRToken();
		IRToken(std::string op, std::string dst="", std::string src="", std::string extra="");

		std::string op(void)	{ return this->_op_;  }
		std::string dst(void)	{ return this->_dst_; }
		std::string src(void)	{ return this->_src_; }
		std::string extra(void)	{ return this->_ext_; }
		friend std::fstream& operator<< (std::fstream& stream, const IRToken &src);
	private:
		std::string _op_, _dst_, _src_, _ext_;
};


class IR : public Binary {
	public:
		IR(std::string dst, off_t entry = 0x1000, bool pie=false, bool symb=false);
		~IR(void);

		void emit(IRToken token);
		void emit(std::string op, std::string &dst, std::string &src, std::string &extra);

		virtual std::string regalloc(std::string src) = 0;
		virtual std::string tmpreg(void) = 0;
	protected:
		bool _only_ir_;
		std::string randstr(int size=24, std::string prefix="__", std::string suffix="__");
		std::vector<std::string> _repeate_label_;
	private:
		int _param_nr_;
		bool _symb_;
		off_t _entry_;
		std::string _param_[MAX_ZASMP_PARAM];
};

#endif /* __ZERG_IR_H__ */
