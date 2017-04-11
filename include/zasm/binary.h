/* Copyright (C) 2014-2017 cmj. All right reserved. */
#ifndef __ZASM_BINARY_H__
#  define __ZASM_BINARY_H__

#include <vector>
class Binary {
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
};
#endif /* __ZASM_BINARY_H__ */
