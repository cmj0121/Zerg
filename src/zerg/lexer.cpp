/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include "zerg.h"

/* Private Properties */
void Zerg::lexer(std::string src) {
	std::string line;
	std::fstream fs(src);
	ZergToken token, prev("\n");

	if (!fs.is_open()) {
		/* cannot open the source code */
		_D(LOG_CRIT, "source file `%s` does NOT exist", src.c_str());
	}

	while (std::getline(fs, line)) {
		for (size_t cur = 0; cur <= line.size(); ++cur) {
			size_t pos;
			int base = 10;

			switch(line[cur]) {
				case '\0':				/* NEWLINE */
				case '#' :				/* COMMENT */
					cur = line.size();
					token = "\n";
					prev  = this->parser(token, prev);
					break;
				case ' ': case '\t':	/* SPACE */
					/* FIXME - Maybe INDENT or DEDENT */
					break;
				case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': case '8':
				case '9': case '0':
					for (pos = cur; pos <= line.size(); ++pos) {
						if (10 == base && line[pos] >= '0' && line[pos] <= '9') {
							/* NUMBER */
							continue;
						} else if (2 == base && line[pos] >= '0' && line[pos] <= '1') {
							/* NUMBER */
							continue;
						} else if (8 == base && line[pos] >= '0' && line[pos] <= '7') {
							/* NUMBER */
							continue;
						} else if (16 == base && ((line[pos] >= '0' && line[pos] <= '9') ||
										((line[pos] | 0x20) >= 'a' && (line[pos] | 0x20) <= 'f'))) {
							/* NUMBER */
							continue;
						} else if (pos == cur+1) {
							if (line[pos] == 'x' || line[pos] == 'X') {
								base = 16;
								continue;
							} else if (line[pos] == 'b' || line[pos] == 'B') {
								base = 2;
								continue;
							} else if (line[pos] == 'o' || line[pos] == 'O') {
								base = 8;
								continue;
							}
						}
						break;
					}
					token = line.substr(cur, pos-cur);
					prev = this->parser(token, prev);
					cur = pos - 1;
					break;
				case '+': case '-':	case '>': case '<':	/* OPERATOR */
				case '*': case '/': case '%': case '~':
					for (pos = cur; pos <= line.size(); ++pos) {
						if ('+' == line[pos] || '-' == line[pos]) {
							continue;
						} else if ('<' == line[pos] || '>' == line[pos]) {
							continue;
						} else if ('*' == line[pos] || '/' == line[pos]) {
							continue;
						} else if ('%' == line[pos] || '~' == line[pos]) {
							continue;
						}
						break;
					}
					token = line.substr(cur, pos-cur);
					prev = this->parser(token, prev);
					cur = pos - 1;
					break;
				case '.': case ',': case ':':			/* OPERATOR */
				case '(': case ')': case '[': case ']':
				case '{': case '}':
					token = line.substr(cur, 1);
					prev = this->parser(token, prev);
					break;
				case '\'': case '"':					/* STRING */
					for (pos = cur+1; pos <= line.size(); ++pos) {
						if (line[pos] == line[cur]) break;
					}

					if (line[pos] != line[cur]) {
						_D(LOG_CRIT, "Invalid syntax for string %s", line.c_str());
					}
					token = line.substr(cur, pos-cur+1);
					prev = this->parser(token, prev);
					cur = pos;
					break;
				default:
					for (pos = cur+1; pos <= line.size(); ++pos) {
						if ((line[pos] | 0x20) >= 'a' && (line[pos] | 0x20) <= 'z') {
							/* Alphabet */
							continue;
						} else if (line[pos] >= '0' && line[pos] <= '9') {
							/* NUMBER */
							continue;
						} else if (line[pos] == '_') {
							/* UNDERLINE */
							continue;
						}
						break;
					}
					token = line.substr(cur, pos-cur);
					prev = this->parser(token, prev);
					cur = pos - 1;
					break;
			}
		}
	}
}
