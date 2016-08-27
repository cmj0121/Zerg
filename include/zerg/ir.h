/* Copyright (C) 2016-2016 cmj. All right reserved. */

#ifndef __ZERG_IR_H__
#	define __ZERG_IR_H__

#include <vector>
#include <fstream>
#include <string>

#include "zasm.h"

class Tuple {
	public:
		Tuple(std::string op, std::string dst="", std::string x="", std::string y="");
		friend std::fstream& operator << (std::fstream &stream, const Tuple &src);
	private:
		std::string _op_, _dst_, _x_, _y_;
};

class IR : public Binary {
	public:
		IR(std::string dst);
	private:
		std::vector<std::string> _operators_;
};

#endif /* __ZERG_IR_H__ */
