/* Copyright (C) 2014-2017 cmj. All right reserved. */
#include "zasm.h"

Binary::Binary(std::string src, bool pie) : _pie_(pie), _src_(src), _inst_() {
};
Binary::~Binary() {
	for (unsigned int i=0; i<_inst_.size(); ++i) {
		delete _inst_[i];
		_inst_[i] = NULL;
	}
}


void Binary::reallocreg(void) {
	for (unsigned int idx = 0; idx < _inst_.size(); ++idx) {
		off_t offset = 0;

		if (!_inst_[idx]->readdressable()) {
			continue;
		}

		for(unsigned int pos = idx+1; pos < _inst_.size(); ++pos) {
			if (_inst_[pos]->label() == _inst_[idx]->refer()) {
				pos --;
				while (pos > idx) {
					offset += _inst_[pos]->length();
					pos --;
				}
				goto END;
			}
		}

		for(int pos = idx-1; pos >= 0; --pos) {
			if (_inst_[pos]->label() == _inst_[idx]->refer()) {
				/* HACK - Fix the compiler warning sign-compare */
				while (pos < (int)idx) {
					offset -= _inst_[pos+1]->length();
					pos ++;
				}
				goto END;
			}
		}

		_D(LOG_CRIT, "Not found the symbol [%s]", _inst_[idx]->refer().c_str());
		exit(-1);
END:
		_inst_[idx]->setIMM(offset, 4, true);
	}
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

	for (unsigned int i = 0; i < _inst_.size(); ++i) {
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


/* vim set: tabstop=4 */

