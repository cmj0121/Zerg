/* Copyright (C) 2014-2017 cmj. All right reserved. */
#ifndef __ZASM_BINARY_H__
#  define __ZASM_BINARY_H__

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
		Binary(std::string src, bool pie=false);
		virtual ~Binary();

		/* binary-specified function */
		off_t dump(off_t entry = 0x1000, bool symb=false);

		void reallocreg(void);
		off_t length(void);
		off_t nrInst(void);
		void insert(Instruction* inst, int pos);
		Instruction *getInst(int pos);
		std::string get(int pos);
		Binary& operator+= (Instruction *inst);
	private:
		bool _pie_;
		std::string  _src_;
		std::fstream _bin_;
		std::vector<Instruction *> _inst_;
		std::vector<std::string> _symb_;
};
#endif /* __ZASM_BINARY_H__ */
