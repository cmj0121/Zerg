/* Copyright (C) 2014-2017 cmj. All right reserved. */

#include <unistd.h>
#include "zasm.h"

void Zasm::compile(std::string srcfile) {
	off_t entry = 0x100000;
	std::vector<ZasmToken *> line;
	ZasmToken *token = NULL;
	std::fstream src(srcfile, std::fstream::in);

	while (*(token = this->token(src)) != "") {
		if (*token == TOKEN_ENTRY) {
			entry = this->token(src)->asInt();
			line.clear();
		} else if (*token == ZASM_INCLUDE) {
			std::string file = this->token(src)->unescape();

			_D(ZASM_LOG_INFO, "load external file `%s`", file.c_str());
			if (0 != access(file.c_str(), F_OK)) {
				_D(LOG_CRIT, "external file `%s` does NOT exist", file.c_str());
			}

			this->compile(file);
		} else if (*token == "\n") {
			for (size_t i = 0; i < line.size(); ++i) {
				if (*line[i] == ZASM_MEM_BYTE  || *line[i] == ZASM_MEM_WORD ||
					*line[i] == ZASM_MEM_DWORD || *line[i] == ZASM_MEM_QWORD) {

					delete line[i];
					line[i] = new ZasmToken(line[i]->raw() + " " + line[i+1]->raw());

					line.erase(line.begin()+i+1);
					break;
				}
			}

			switch (line.size()) {
				case 0:
					/* NOP */
					break;
				case 1:
					(*this) += new Instruction(line[0]);
					break;
				case 2:
					(*this) += new Instruction(line[0], line[1]);
					break;
				case 3:
					(*this) += new Instruction(line[0], line[1], line[2]);
					break;
				default:
					_D(LOG_CRIT, "Not Implemented %zu", line.size());
					exit(-1);
					break;
			}
			line.clear();
		} else {
			line.push_back(token);
		}
	}

	/* Output the binary */
	this->dump(entry, this->_args_.symbol);
}
ZasmToken* Zasm::token(std::fstream &src) {
	char ch;
	std::string token = "";

	while (EOF != (ch = src.get())) {
		switch (ch) {
			case '#':				/* Ignore any comments */
				while (EOF != (ch = src.get())) {
					if ('\n' == ch) {
						src.putback(ch);
						break;
					}
				}
				continue;
			case '\n':				/* Always reply newline */
				this->_linono_ ++;
				token = '\n';
				break;
			case '\t': case ' ':	/* Ignore any white-space */
				continue;
			case '"': case '\'':	/* Special case - string */
				token = ch;
				while(EOF != (ch = src.get()) && ch != token[0]) {
					token += ch;
				}
				token += ch;
				break;
			case '[':				/* Special case - memory */
				token = ch;
				while(EOF != (ch = src.get()) && '\n' != ch) {
					token += ch;
					if (']' == ch) {
						break;
					}
				}
				if (']' != token[token.size()-1]) {
					_D(LOG_CRIT, "Syntax Error - '%s'", token.c_str());
					exit(-1);
				}
				break;
			default:
				token = ch;
				while(EOF != (ch = src.get())) {
					if (' ' == ch || '\t' == ch || '\n' == ch) {
						src.putback(ch);
						break;
					}
					token += ch;
				}
				break;
		}
		break;
	}
	return new ZasmToken(token);
}
