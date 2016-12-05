/* Copyright (C) 2014-2016 cmj. All right reserved. */

#ifdef __APPLE__

#include <sys/stat.h>
#include "zasm.h"

#include "macho64_inc.hpp"

Binary::Binary(std::string src, bool pie) : _pie_(pie), _src_(src), _inst_() {
};
Binary::~Binary() {
	for (int i=0; i<_inst_.size(); ++i) {
		delete _inst_[i];
		_inst_[i] = NULL;
	}
}

off_t Binary::dump(off_t entry, bool showSymb) {
	int size = this->length();
	off_t header_offset = 0;

	/* Reallocate all address if need */
	for (int idx = 0; idx < _inst_.size(); ++idx) {
		int pos;
		off_t offset = 0;

		if (!_inst_[idx]->readdressable()) {
			continue;
		}

		for(pos =idx+1; pos< _inst_.size(); ++pos) {
			if (_inst_[pos]->label() == _inst_[idx]->refer()) {
				break;
			}
		}
		if (pos == _inst_.size()) {
			for(pos =idx-1; pos >= 0; --pos) {
				if (_inst_[pos]->label() == _inst_[idx]->refer()) {
					break;
				}
			}
		}

		if (pos == _inst_.size() || pos == idx || 0 > pos) {
			_D(LOG_CRIT, "Not found the symbol [%s]", _inst_[idx]->refer().c_str());
			exit(-1);
		} else if (pos > idx) {
			for (int i = idx+1; i < pos; ++i) offset += _inst_[i]->length();
		} else {
			for (int i = idx; i > pos; --i) offset -= _inst_[i]->length();
		}

		_inst_[idx]->setIMM(offset, 4, true);
	}
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

				if ("" != symb && '_' != symb[0]) {
					struct nlist_64 symlist;

					symlist.n_un.n_strx = symboff;
					symlist.n_type		= N_TYPE;
					symlist.n_sect		= 0x01;
					symlist.n_desc		= REFERENCE_FLAG_UNDEFINED_NON_LAZY;
					symlist.n_value		= binoff;
					this->_symb_.push_back(std::make_pair(symb,  symlist));

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
		for (auto it : this->_symb_) {
			_bin_.write((char *)&it.second, sizeof(struct nlist_64));
		}
		_bin_.write("\x00", 1);
		for (auto it : this->_symb_) {
			_bin_.write(it.first.c_str(), it.first.size());
			_bin_.write("\x00", 1);
		}
	}

	_bin_.close();

	chmod(_src_.c_str(), 0755);
	return (off_t)0;
}

void Binary::insert(Instruction* inst, int pos) {
	this->_inst_.insert(this->_inst_.begin() + pos, inst);
}
std::string Binary::get(int pos) {
	return this->_inst_[pos]->show();
}
Binary& Binary::operator+= (Instruction *inst) {
	this->_inst_.push_back(inst);
	return *this;
}
off_t Binary::length(void) {
	off_t len = 0;

	for (int i = 0; i < _inst_.size(); ++i) {
		len += _inst_[i]->length();
	}

	return len;
};
off_t Binary::nrInst(void) {
	return this->_inst_.size();
}
Instruction *Binary::getInst(int pos) {
	return this->_inst_[pos];
}

#endif /* __APPLE__ */
/* vim set: tabstop=4 */

