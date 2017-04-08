/* Copyright (C) 2014-2017 cmj. All right reserved. */
#ifndef __ZASM_BINARY_H__
#  define __ZASM_BINARY_H__

#define VALID_SYMBOL(_symb_)	\
	(_symb_ == ZASM_ENTRY_POINT || ("" != _symb_ && '.' != _symb_[0] && '_' != _symb_[0]))

class Utils {
	public:
		virtual ~Utils();

		bool  isInt(std::string src);
		off_t toInt(std::string src);

		std::string unescape(std::string src);
		std::string toHex(unsigned char *payload, size_t size);
};
#include <vector>
class Binary : public Utils {
	public:
		Binary(std::string dst, bool pie=false) : _pie_(pie), _dst_(dst) {};
		virtual ~Binary();

		void dump(off_t entry = 0x1000, bool symb=false);	/* Binary-Specified */
		void reallocreg(void);
		off_t length(void);

		Binary& operator+= (Instruction *inst);
	private:
		bool _pie_;
		std::string _dst_;
		std::vector<Instruction *> _inst_;
		std::vector<std::string> _symb_;
};
#endif /* __ZASM_BINARY_H__ */
