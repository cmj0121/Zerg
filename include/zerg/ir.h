/* Copyright (C) 2016-2016 cmj. All right reserved. */

#ifndef __ZERG_IR_H__
#	define __ZERG_IR_H__

#include <vector>
#include <fstream>
#include <string>

#include "zasm.h"

typedef std::string		ZasmPToken;
#define MAX_ZASMP_PARAM	10
/* Low-Level IR to emit Zasm language
 *
 * This IR is a max-to-4 tuple (OP, DST, SRC, EXTRA) and support 40 pseudo operators
 */
class ZasmPEmitter : public Binary {
	public:
		ZasmPEmitter(std::string dst);
		void emit(ZasmPToken op, ZasmPToken dst="", ZasmPToken src="", ZasmPToken extra="");
	private:
		int _param_nr_;;
		ZasmPToken _param_[MAX_ZASMP_PARAM];
};

class IR : public ZasmPEmitter {
	public:
		IR(std::string dst);

};

#endif /* __ZERG_IR_H__ */
