/* Copyright (C) 2014-2016 cmj. All right reserved. */

#ifdef __APPLE__

#include <sys/stat.h>
#include "zasm.h"

#include "macho64_inc.hpp"

off_t Binary::dump(off_t entry, bool showSymb) {
	int size = this->length();
	off_t header_offset = 0;
	std::vector<std::pair<std::string, struct nlist_64>> symblist;

	this->reallocreg();
	_bin_.open(_src_, std::fstream::out | std::fstream::binary | std::fstream::trunc);

	/* Create necessary header, dummy first */
	for (int i = 0; i < 2; ++i) {
		off_t symboff = 1, binoff = 0;

		header(_bin_, 8, header_offset, this->_pie_);

		seg_pagezero(_bin_);
		seg_text(_bin_, size, entry, header_offset);
		seg_linkedit(_bin_, this->_symb_);
		dyld_info(_bin_);
		seg_symtab(_bin_, this->_symb_);
		seg_dysymtab(_bin_);
		dyld_link(_bin_);
		seg_unixthread(_bin_, entry, header_offset);

		header_offset 	= _bin_.tellg();
		binoff			= entry + header_offset;

		if (true == showSymb && 0 == i) {
			for (int idx = 0; idx < _inst_.size(); ++idx) {
				std::string symb = _inst_[idx]->label();

				if (_inst_[idx]->isLabel() && VALID_SYMBOL(symb)) {
					struct nlist_64 symlist;

					symlist.n_un.n_strx = symboff;
					symlist.n_type		= N_TYPE;
					symlist.n_sect		= 0x01;
					symlist.n_desc		= REFERENCE_FLAG_UNDEFINED_NON_LAZY;
					symlist.n_value		= binoff;

					symblist.push_back(std::make_pair(symb,  symlist));
					this->_symb_.push_back(symb);

					symboff += symb.size() + 1;
				}

				binoff += _inst_[idx]->length();
			}
		}
	}

	/* Write machine code */
	for (int idx = 0; idx < _inst_.size(); ++idx) {
		(*_inst_[idx]) << _bin_;
	}

	/* HACK - Mach-O 64 always need large than 4K */
	while (0x1000 > _bin_.tellg()) {
		_bin_.write("\x00", 1);
	}

	/* dump the symbol */
	if (showSymb) {
		for (auto it : symblist) {
			_bin_.write((char *)&it.second, sizeof(struct nlist_64));
		}
		_bin_.write("\x00", 1);
		for (auto it : symblist) {
			_bin_.write(it.first.c_str(), it.first.size());
			_bin_.write("\x00", 1);
		}
	}

	_bin_.close();

	chmod(_src_.c_str(), 0755);
	return (off_t)0;
}

#endif /* __APPLE__ */
/* vim set: tabstop=4 */

