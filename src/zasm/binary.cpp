/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <sys/stat.h>
#include "zasm.h"

void Zasm::dump_bin(off_t entry, bool showSymb) {
	std::fstream fp;

	Zasm::reallocreg();
	fp.open(this->_dst_, std::fstream::out | std::fstream::binary | std::fstream::trunc);

	/* Write machine code */
	for (size_t idx = 0; idx < _inst_.size(); ++idx) {
		(*_inst_[idx]) << fp;
	}

	fp.close();
	chmod(this->_dst_.c_str(), 0755);
}
