/* Copyright (C) 2014-2017 cmj. All right reserved. */

#include <unistd.h>
#include "zasm.h"

void Zasm::assemble(std::string srcfile) {
	std::fstream src(srcfile, std::fstream::in);
	std::string line;
	std::vector<std::string> inst;

	_D(LOG_ZASM_INFO, "assemble zasm source `%s`", srcfile.c_str());
	while (std::getline(src, line)) {
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
				_D(LOG_DEBUG_LEXER, "zasm token `%s`", tmp.c_str());

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
			continue;
		} else if (ZASM_INCLUDE == inst[0]) {
			if (2 != inst.size()) {
				/* syntax error */
				_D(LOG_CRIT, "syntax error `include [source file]` #%zu", inst.size());
			}
			this->assemble(inst[1]);
		} else if (ZASM_ENTRY_ADDR == inst[0]) {
			if (2 != inst.size()) {
				_D(LOG_CRIT, "syntax error `include [entry address]`");
			}
			this->_args_.entry = InstToken(inst[1]).asInt();
		} else if (ZASM_DEFINE == inst[0]) {
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
		} else {
			switch(inst.size()) {
				case 0:
					/* NOP*/
					break;
				case 1:
					(*this) += new Instruction(inst[0]);
					break;
				case 2:
					(*this) += new Instruction(inst[0], inst[1]);
					break;
				case 3:
					(*this) += new Instruction(inst[0], inst[1], inst[2]);
					break;
				default:
					_D(LOG_CRIT, "Not Support instruction - `%s`", line.c_str());
					break;
			}
		}

		/* tear-down */
		inst.clear();
	}
}

