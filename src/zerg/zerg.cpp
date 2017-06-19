/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iostream>
#include <iomanip>

#include <unistd.h>
#include "zerg.h"

Zerg::Zerg(std::string dst, Args &args) : IR(dst, args), _args_(args) {
	this->_lineno_		= 1;
	this->_args_		= args;
}
Zerg::~Zerg() {
}

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
