/* Copyright (C) 2014-2017 cmj. All right reserved. */

#include <iostream>
#include <iomanip>
#include <set>

#include <unistd.h>
#include "zerg.h"


void Zerg::compile(std::string src) {
	CFG *node = NULL;

	if (this->_args_.compile_ir) {
		IR::compile(src);
	} else {
		/* Set the entry point of the program */
		this->emit(IR_LABEL, ZASM_ENTRY_POINT);
		this->emit(IR_CONDITION_CALL, __IR_REFERENCE__ ZASM_MAIN_FUNCTION);
		this->emit(IR_MEMORY_PUSH, "0x2000001");
		this->emit(IR_INTERRUPT);

		/* load the built-in library if possible */
		if (0 == access(BUILTIN_LIBRARY, F_OK) && false == this->_args_.no_stdlib) {
			_D(LOG_INFO, "Load the built-in library `%s`", BUILTIN_LIBRARY);
			//this->parser(BUILTIN_LIBRARY);
		}

		node = new CFG(Parser::parser(src));
		this->emit(node, ZASM_MAIN_FUNCTION);
		this->emit(IR_CONDITION_RET);

		/* dump all globals symbol */
		for (auto it : this->globals_str) {
			/* FIXME - Should we need globals defined? of just set in local part */
			this->emit(IR_DEFINE, it.first, it.second);
		}
	}
}

void Zerg::emit(CFG *cfg, std::string name) {
	char buff[BUFSIZ] = {0};
	std::set<std::string> locals = {};
	AST  *node = cfg->ast();

	#if defined(DEBUG_CFG) || defined(DEBUG)
		std::cerr << "==== CFG node ====" << std::endl;
		std::cerr << *node << std::endl;
	#endif /* DEBUG_CFG */

	/* calculate the number of local variables */
	if (ZTYPE_UNKNOWN == node->type()) {
		for (int i = 0; i < node->length(); ++i)
			switch(node->child(i)->type()) {
				case ZTYPE_RASSIGN:
				case ZTYPE_LASSIGN:
					locals.insert(node->child(i)->child(0)->raw());
					break;
				default:
					break;
			}
	}
	snprintf(buff, sizeof(buff), "0x%lX", locals.size()*PARAM_SIZE);

	this->emit(IR_LABEL, name);
	this->emit(IR_PROLOGUE, buff);
	this->emitIR(node);
	this->emit(IR_EPILOGUE, buff);

	switch(cfg->type()) {
		case CFG_BLOCK:
			if (NULL != cfg->transfer()) this->emit(cfg->transfer(), cfg->name());
			break;
		default:
			_D(LOG_CRIT, "Not Implementation");
			break;
	}
}

void Zerg::emit(IROP opcode, std::string dst, std::string src, std::string size) {
	if (this->_args_.only_ir) {
		for (auto it : IROP_map) {
			if (it.second == opcode) {
				std::cout << std::setw(14) << std::left << it.first
							<< std::setw(11) << dst
							<< std::setw(11) << src
							<< std::setw(11) << size << std::endl;
				return ;
			}
		}
		_D(LOG_CRIT, "Cannot found opcode #%d", opcode);
	} else {
		return IR::emit(opcode, dst, src, size);
	}
}
