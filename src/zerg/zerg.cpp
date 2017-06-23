/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iostream>
#include <iomanip>

#include <unistd.h>
#include "zerg.h"


void Zerg::compile(std::string src) {
	AST *ast = NULL;
	if (this->_args_.compile_ir) {
		IR::compile(src);
	} else {
		/* load the built-in library if possible */
		if (0 == access(BUILTIN_LIBRARY, F_OK) && false == this->_args_.no_stdlib) {
			_D(LOG_INFO, "Load the built-in library `%s`", BUILTIN_LIBRARY);
			//this->parser(BUILTIN_LIBRARY);
		}
		ast = this->parser(src);
	}
}

AST* Zerg::parser(std::string srcfile) {
	AST *node = Parser::parser(srcfile);

	/* generate the CFG is possible */

	return node;
}
