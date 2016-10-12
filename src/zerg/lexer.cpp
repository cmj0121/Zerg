/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include "zerg.h"

/* Private Properties */
void Zerg::lexer(std::string src) {
	std::string line;
	std::fstream fs(src);
	AST *ast = NULL;
	CFG *cfg = NULL, *curcfg = NULL;

	if (!fs.is_open()) {
		/* cannot open the source code */
		_D(LOG_CRIT, "source file `%s` does NOT exist", src.c_str());
	}

	while (std::getline(fs, line)) {
		TOKENS _tokens;

		for (size_t cur = 0; cur <= line.size(); ++cur) {
			size_t pos;
			_D(LOG_DEBUG, "read line `%s` on %zu #%zu", line.c_str(), cur, _tokens.size());

			switch(line[cur]) {
				case '\0':				/* NEWLINE */
				case '#' :				/* COMMENT */
					goto NEXT_STEP;
					break;
				case ' ': case '\t':	/* SPACE */
					/* FIXME - Maybe INDENT or DEDENT */
					break;
				case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': case '8':
				case '9': case '0':
					for (pos = cur; pos <= line.size(); ++pos) {
						if (line[pos] >= '0' && line[pos] <= '9') {
							/* NUMBER */
							continue;
						} else if (pos == cur+1) {
							if (line[pos] == 'x' || line[pos] == 'X') {
								continue;
							} else if (line[pos] == 'o' || line[pos] == 'O') {
								continue;
							}
						}
						break;
					}
					_tokens.push_back(line.substr(cur, pos-cur));
					cur = pos - 1;
					break;
				case '+': case '-':						/* OPERATOR */
				case '*': case '/': case '%': case '~':	/* OPERATOR */
				case '.': case ',': case ':':
				case '(': case ')': case '[': case ']':
				case '{': case '}':
					_tokens.push_back(line.substr(cur, 1));
					break;
				case '\'': case '"':					/* STRING */
					for (pos = cur+1; pos <= line.size(); ++pos) {
						if (line[pos] == line[cur]) break;
					}

					if (line[pos] != line[cur]) {
						_D(LOG_CRIT, "Invalid syntax for string %s", line.c_str());
					}
					_tokens.push_back(line.substr(cur, pos-cur+1));
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
					_tokens.push_back(line.substr(cur, pos-cur));
					cur = pos - 1;
					break;
			}
		}

		NEXT_STEP:
			std::vector<ZergToken> tokens;

			if (0 != _tokens.size()) {
				for (auto it : _tokens) {
					ZergToken tmp(it);

					tokens.push_back(tmp);
				}

				ALERT(NULL == (ast = this->parser(tokens)));

				cfg = new CFG("");
				cfg->insert(ast);

				if (0 == this->_root_.size()) {
					this->_root_[CFG_MAIN] = cfg;
					curcfg = cfg;
				} else {
					curcfg->passto(cfg);
					curcfg = cfg;
				}
			}
	}
}
