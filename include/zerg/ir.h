/* Copyright (C) 2016-2016 cmj. All right reserved. */

#ifndef __ZERG_IR_H__
#	define __ZERG_IR_H__

#include <vector>
#include <fstream>
#include <string>

#include "zasm.h"

#define MAX_ZASMP_PARAM	10
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
		IR(std::string dst);

		void emit(IRToken token);
		void emit(std::string op, std::string dst="", std::string src="", std::string extra="");
	private:
		int _param_nr_;;
		std::string _param_[MAX_ZASMP_PARAM];
};

#endif /* __ZERG_IR_H__ */
