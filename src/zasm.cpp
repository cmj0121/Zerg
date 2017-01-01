/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iostream>
#include <string>
#include <vector>

#include <getopt.h>

#include "zasm.h"

int  __verbose__ = 0;
bool __pie__     = false;
bool __symbol__  = false;

void Zasm::compile(std::fstream &src, bool symb) {
	off_t entry = 0x1000;
	std::vector<ZasmToken *> line;
	ZasmToken *token = NULL;

	while (*(token = this->token(src)) != "") {
		if (*token == TOKEN_ENTRY) {
			entry = this->token(src)->asInt();
			line.clear();
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
	this->dump(entry, symb);
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
				__line__ ++;
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
void help(void) {
	fprintf(stderr, "ZASM - Zerg assembler\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage - zasm [option] src\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Option\n");
	fprintf(stderr, "    -v, --verbose          Show the verbose message\n");
	fprintf(stderr, "    -o, --output <file>    Write output to <file>\n");
	fprintf(stderr, "        --pie              PIE - Position-Independent Executables\n");
	fprintf(stderr, "        --symbol           Show the symbol information\n");
	exit(-1);
}
int main(int argc, char *argv[]) {
	int optIdx = -1;
	char ch, opts[] = "vo:";
	struct option options[] = {
		{"output",  optional_argument,	0, 'o'},
		{"pie",		no_argument,		0, 'p'},
		{"symbol",	optional_argument,	0, 'S'},
		{"verbose",	optional_argument,	0, 'v'},
		{NULL, 0, 0, 0}
	};
	std::string dst = "a.out";
	std::fstream src;

	while (-1 != (ch = getopt_long(argc, argv, opts, options, &optIdx))) {
		switch(ch) {
			case 'o':
				dst = optarg;
				break;
			case 'p':
				switch(optIdx) {
					case 1:		/* --pie */
						__pie__ = true;
						break;
					default:
						_D(LOG_CRIT, "Not Implemented");
						break;
				}
				break;
			case 'S':
				switch(optIdx) {
					case 2:		/* --symbol */
						__symbol__ = true;
						break;
					default:
						_D(LOG_CRIT, "Not Implemented");
						break;
				}
				break;
			case 'v':
				__verbose__++;
				break;
		}
	}
	argc -= optind;
	argv += optind;

	if (0 == argc) {
		help();
	}

	do {
		std::fstream src;
		std::string token;
		std::vector<std::string> line;

		if (0 >= argc) {
			_D(LOG_CRIT, "Bad Parameter");
			exit(-1);
		}

		src.open(argv[0], std::fstream::in);

		Zasm *bin = new Zasm(dst, __pie__);
		bin->compile(src, __symbol__);

		delete bin;
	} while (0);

	return 0;
}
/* vim set: tabstop=4 */

