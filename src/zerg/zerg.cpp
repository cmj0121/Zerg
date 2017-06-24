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

	/* emit the IR from CFG-AST */
	for (ssize_t i = 0; i < node->length(); ++i) {
		this->emitIR(node->child(i));
	}
	return node;
}
void Zerg::emitIR(AST *node) {
	AST *sub = NULL;
	ALERT(NULL == node);

	_D(LOG_DEBUG_IR, "emit IR on %s", node->raw().c_str());
	switch(node->type()) {
		case ZTYPE_CMD_NOP:
			ALERT(0 != node->length());
			IR::emit(IR_NOP);
			break;
		case ZTYPE_CMD_ASM:
			sub = node->child(0);
			ALERT(ZTYPE_STRING != sub->type() || 0 != sub->length());
			IR::emit("INLINE", sub->raw(), "", "");
			break;
		default:
			_D(LOG_CRIT, "Not implemented on #%d", node->type());
			break;
	}
}
