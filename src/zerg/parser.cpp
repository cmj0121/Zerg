/* Copyright (C) 2017-2017 cmj. All right reserved. */

#include <string>
#include <fstream>

#include "zerg.h"

/* Parse the source file and generate the CFG by following steps
 *
 *     1. Lexer by function `lexer`
 *     2. Parse and generate a big AST
 *     3. Split into several AST and generate a CFG
 */
void Zerg::parser(std::string srcfile) {
	std::string line;
	std::fstream fs(srcfile);
	ZergToken prev = {"", ZTYPE_NEWLINE}, token;

	if (!fs.is_open()) {
		/* cannot open the source code */
		_D(LOG_CRIT, "source file `%s` does NOT exist", srcfile.c_str());
	}

	this->_lineno_  = 0;
	this->_srcfile_ = srcfile;

	while ((token = this->lexer(fs)).second != ZTYPE_UNKNOWN) {
		if (prev.second == ZTYPE_NEWLINE && token.second == ZTYPE_NEWLINE) {
			/* Skip continuously NEWLINE */
			continue;
		}

		_D(LOG_DEBUG_LEXER, " -> token - #0x%-4X %s", token.second, token.first.c_str());
		/* ... */
		prev = token;
	}
}
