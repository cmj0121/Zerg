/* Copyright (C) 2014-2017 cmj. All right reserved. */

#include "zasm.h"

Zasm::~Zasm(void) {
	for (unsigned int i=0; i<_inst_.size(); ++i) {
		delete _inst_[i];
		_inst_[i] = NULL;
	}
}
void Zasm::assembleF(std::string srcfile) {
	std::fstream src(srcfile, std::fstream::in);
	std::string line;

	_D(LOG_ZASM_LEXER, "assemble zasm source `%s`", srcfile.c_str());
	while (std::getline(src, line)) {
		Zasm::assembleL(line);
	}
}
void Zasm::assembleL(std::string line) {
	std::vector<std::string> inst;
	_D(LOG_ZASM_DEBUG, "assemble `%s`", line.c_str());

	/* lexer analysis */
	for (size_t i = 0; i <= line.size(); ++i) {
		std::string tmp = "";

		switch(line[i]) {
			case '\0': case '\n': case '#':	/* End-Of-Instruction */
				this->_linono_ ++;
				goto ASSEMBLE;
			case ' ': case '\t':			/* White Space */
				/* IGNORE */
				continue;
			case '"': case '\'':			/* String */
				tmp = line[i++];
				while ('\0' != line[i]) {
					tmp += line[i];

					if (tmp[0] == line[i]) break;
					++i;
				}

				if (tmp[0] != line[i]) {
					/* invalid syntax for string */
					_D(LOG_CRIT, "syntax error - %s (L#%d)", line.c_str(), _linono_);
				}
				break;
			case '[':						/* Memory */
				tmp = line[i++];
				while ('\0' != line[i]) {
					tmp += line[i];

					if (']' == line[i]) break;
					++i;
				}

				if (']' != line[i]) {
					/* invalid syntax for memory */
					_D(LOG_CRIT, "syntax error - %s (L#%d)", line.c_str(), _linono_);
				}
				break;
			default:
				do {
					/* simple token */
					tmp += line[i++];
				} while (! ('\0' == line[i] || ' ' == line[i] || '\t' == line[i]));
				break;
		}

		if ("" != tmp) {
			std::string prev = 0 == inst.size() ? "" : inst[inst.size()-1];

			/* show the zasm token */
			_D(LOG_ZASM_DEBUG, "zasm token `%s`", tmp.c_str());

			if (ZASM_MEM_BYTE  == prev || ZASM_MEM_WORD  == prev||
				ZASM_MEM_DWORD == prev || ZASM_MEM_QWORD == prev) {
				inst[inst.size()-1] += " " + tmp;
			} else {
				inst.push_back(tmp);
			}
		}
	}


	ASSEMBLE: /* dump the machine code */

	for (size_t i = 1; i < inst.size(); ++i) {
		if (InstToken(inst[i]).isREF() && 0 != this->_map_.count(inst[i].substr(1))) {
			if ("" != this->_map_[inst[i].substr(1)]) {
				_D(LOG_ZASM_INFO, "replace `%s` -> `%s`", inst[i].c_str(),
											this->_map_[inst[i].substr(1)].c_str());
				inst[i] = this->_map_[inst[i].substr(1)];
			}
		}
	}

	if (0 == inst.size()) {
		/* NOP */
	} else if (ZASM_ENTRY_ADDR == inst[0]) {
		if (2 != inst.size() || !InstToken(inst[1]).isIMM()) {
			_D(LOG_CRIT, "syntax error - %s [imm]", ZASM_ENTRY_ADDR);
		}

		this->_args_.entry = InstToken(inst[1]).asInt();
		_D(LOG_ZASM_INFO, "reset entry address 0x" OFF_T, this->_args_.entry);
	} else if (ZASM_INCLUDE    == inst[0]) {
		if (2 != inst.size()) {
			/* syntax error */
			_D(LOG_CRIT, "syntax error `include [source file]` #%zu", inst.size());
		}
		Zasm::assembleF(inst[1]);
	} else if (ZASM_DEFINE     == inst[0]) {
		if (3 != inst.size()) {
			/* syntax error */
			_D(LOG_CRIT, "syntax error `define [key] [value]`");
		} else if (0 != this->_map_.count(inst[1])) {
			/* duplicate variable claim*/
			_D(LOG_CRIT, "duplicate claim variable `%s`", inst[1].c_str());
		}

		this->_map_[inst[1]] = "";
		if (InstToken(inst[2]).isIMM()) {
			this->_map_[inst[1]] = inst[2];
		} else {
			(*this) += new Instruction(inst[0], inst[1], inst[2]);
		}
	} else if (ZASM_REPEAT     == inst[0]) {
		if (3 > inst.size()) _D(LOG_CRIT, "syntax error - repeat data times");

		/* FIXME */
		_D(LOG_ZASM_INFO, "repeat %s %s", inst[1].c_str(), inst[2].c_str());
		(*this) += new Instruction(inst[0], inst[1], inst[2]);
	} else {
		/* process the decorator */
		if (1 < inst.size() && ZASM_DECORATOR == inst[1][0]) {
			#ifdef __x86_64__
			if ("@real" == inst[1] || "@16bits" == inst[1]) {
				_mode_ = X86_REAL_MODE;
			} else if ("@pmode" == inst[1] || "@protected" == inst[1]) {
				_mode_ = X86_PROTECTED_MODE;
			} else {
				_D(LOG_CRIT, "Not implemented decorator %s", inst[1].c_str());
			}
			#endif /* __x86_64__ */
		}

		switch(inst.size()) {
			case 0:
				/* NOP*/
				break;
			case 1:
				(*this) += new Instruction(inst[0], "", "", _mode_);
				break;
			case 2:
				(*this) += new Instruction(inst[0], inst[1], "", _mode_);
				break;
			case 3:
				(*this) += new Instruction(inst[0], inst[1], inst[2], _mode_);
				break;
			default:
				_D(LOG_CRIT, "Not Support instruction - `%s`", line.c_str());
				break;
		}
	}

	_D(LOG_ZASM_LEXER, "assemble `%s` - #%zu", line.c_str(), inst.size());
	/* tear-down */
	inst.clear();
}
void Zasm::dump(void) {
	if ("bin" == this->_args_.fmt) {
		/* raw binary format */
		dump_bin(this->_args_.entry, this->_args_.symbol);
	#if defined(__APPLE__) && defined(__x86_64__)
	} else if ("macho64" == this->_args_.fmt) {
		/* Mach-O 64 platform */
		dump_macho64(this->_args_.entry, this->_args_.symbol);
	#endif	/* __APPLE__ */
	#if defined(__linux__) && defined(__x86_64__)
	} else if ("macho64" == this->_args_.fmt) {
		/* ELF-64 platform */
		dump_elf64(this->_args_.entry, this->_args_.symbol);
	#endif	/* __APPLE__ */
	} else {
		_D(LOG_CRIT, "Not implemented %s", this->_args_.fmt.c_str());
	}
}
Zasm& operator += (Zasm &zasm, Instruction *inst) {
	zasm._inst_.push_back(inst);
	return zasm;
}

/* ======== private methods ======== */
void Zasm::reallocreg(void) {
	_D(LOG_ZASM_DEBUG, "re-allocate register");
	for (unsigned int idx = 0; idx < _inst_.size(); ++idx) {
		off_t offset = 0;

		if (!_inst_[idx]->readdressable()) {
			continue;
		}

		if (_inst_[idx]->isIMMRange()) {
			off_t size = 0;
			InstToken tmp;

			if ("" == _inst_[idx]->rangeFrom()) {
				size = 0;
			} else if (ZASM_CURRENT_POS == _inst_[idx]->rangeFrom()) {
				for (int pos = idx-1; pos >= 0 ; --pos) {
					size -= _inst_[pos]->length();
				}
			} else if (ZASM_SESSION_POS == _inst_[idx]->rangeFrom()) {
				for (int pos = idx-1; pos >= 0 ; --pos) {
					size -= _inst_[pos]->length();
					if ("" != _inst_[pos]->label()) break;
				}
			} else {
				tmp = InstToken(_inst_[idx]->rangeFrom());
				size = -1 * tmp.asInt();
			}

			tmp = InstToken(_inst_[idx]->rangeTo());
			size += tmp.asInt();
			_inst_[idx]->setRepeat(size);
			goto END;
		} else if (ZASM_CURRENT_POS == _inst_[idx]->refer()) {
			offset = -1 * _inst_[idx]->length();
			goto END;
		} else if (ZASM_SESSION_POS == _inst_[idx]->refer()) {
			for(int pos = idx-1; pos >= 0; --pos) {
				if ("" != _inst_[pos]->label()) {
					/* HACK - Fix the compiler warning sign-compare */
					while (pos < (int)idx) {
						offset -= _inst_[pos+1]->length();
						pos ++;
					}
					goto END;
				}
			}
			_D(LOG_CRIT, "No symbol defined");
		} else if (_inst_[idx]->isABSAddress()) {	/* absolute address */
			offset = this->_args_.entry;

			for (unsigned int pos = 0; pos < _inst_.size(); ++pos) {
				if (_inst_[pos]->label() == _inst_[idx]->refer()) break;
				offset += _inst_[pos]->length();
			}

			goto END;
		} else {									/* related address */
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
		}


		_D(LOG_CRIT, "Not found the symbol [%s]", _inst_[idx]->refer().c_str());
		exit(-1);

		END:
		#ifdef __x86_64__
			_inst_[idx]->setIMM(offset, X86_REAL_MODE == _mode_ ? CPU_16BIT : CPU_32BIT, true);
		#else
			_inst_[idx]->setIMM(offset, CPU_32BIT, true);
		#endif /* __x86_64__ */
	}
}
off_t Zasm::length(void) {
	off_t len = 0;

	for (unsigned int i = 0; i < _inst_.size(); ++i) {
		len += _inst_[i]->length();
	}

	return len;
};
