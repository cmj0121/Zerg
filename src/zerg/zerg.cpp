/* Copyright (C) 2014-2017 cmj. All right reserved. */

#include <iostream>
#include <iomanip>
#include <set>

#include <unistd.h>
#include "zerg.h"


void Zerg::compile(std::string src) {
	char buff[BUFSIZ] = {0};
	CFG *node = NULL;

	if (this->_args_.compile_ir) {
		IR::compile(src);
	} else {
		/* Set the entry point of the program */
		this->emit(IR_LABEL, ZASM_ENTRY_POINT);
		this->emit(IR_CONDITION_CALL, __IR_REFERENCE__ ZASM_MAIN_FUNCTION);
		this->emit(IR_MEMORY_PUSH, "0x2000001");
		this->emit(IR_INTERRUPT);
		this->flush();

		/* load the built-in library if possible */
		if (0 == access(BUILTIN_LIBRARY, F_OK) && false == this->_args_.no_stdlib) {
			_D(LOG_INFO, "Load the built-in library `%s`", BUILTIN_LIBRARY);
			//this->parser(BUILTIN_LIBRARY);
		}

		node = new CFG(Parser::parser(src));
		this->emit(node, ZASM_MAIN_FUNCTION);
		snprintf(buff, sizeof(buff), "0x%lX", IR::localvar_len() * PARAM_SIZE);
		this->emit(IR_EPILOGUE, buff);
		this->emit(IR_CONDITION_RET);
		this->flush();

		/* dump all globals symbol */
		for (auto it : this->globals_str) {
			/* FIXME - Should we need globals defined? of just set in local part */
			this->emit(IR_DEFINE, it.first, it.second);
		}
		this->flush();
	}
}

void Zerg::emit(CFG *cfg, std::string name) {
	std::string branch, end_of_branch;
	AST *node = cfg->ast();

	#if defined(DEBUG_CFG) || defined(DEBUG)
		std::cerr << "==== CFG node ====" << std::endl;
		std::cerr << *node << std::endl;
	#endif /* DEBUG_CFG */

	if ("" != name) this->emit(IR_LABEL, name);
	this->emitIR(node);

	switch(cfg->type()) {
		case CFG_BLOCK:
			if (NULL != cfg->transfer()) this->emit(cfg->transfer(), cfg->name());
			break;
		case CFG_BRANCH:
			branch = IR::randstr(8);
			end_of_branch = IR::randstr(8);

			this->emit(IR_CONDITION_JMPIFN, __IR_REFERENCE__ + branch, node->data());
			this->emit(cfg->branch(true), "");
			if (NULL != cfg->branch(false)) this->emit(IR_CONDITION_JMP, __IR_REFERENCE__ + end_of_branch);
			this->emit(IR_LABEL, branch);
			if (NULL != cfg->branch(false)) {
				this->emit(cfg->branch(false), "");
				this->emit(IR_LABEL, end_of_branch);
			}
			break;

		default:
			_D(LOG_CRIT, "Not Implementation");
			break;
	}
}

void Zerg::emit(IROP opcode, std::string dst, std::string src, std::string size) {
	ZergIR ir = {opcode, dst, src, size};

	/* count the local variable */
	if (opcode == IR_MEMORY_STORE && IR_TOKEN_VAR == IR::token(dst)) {
		IR::localvar(dst);
	}
	this->_ir_stack_.push_back(ir);
}
void Zerg::flush(void) {
	bool blFound = false;
	char buff[BUFSIZ] = {0};
	ZergIR prologue;

	if (this->_ir_stack_.size()) {
		snprintf(buff, sizeof(buff), "0x%lX", IR::localvar_len() * PARAM_SIZE);
		prologue.opcode = IR_PROLOGUE;
		prologue.dst    = buff;

		/* PROLOGUE and EPILOGUE */
		this->_ir_stack_.insert(this->_ir_stack_.begin()+1, prologue);

		_D(LOG_DEBUG_IR, "flush #%lu IR", this->_ir_stack_.size());
		for (auto ir : this->_ir_stack_) {
			if (this->_args_.only_ir) {
				for (auto it : IROP_map) {
					if (it.second == IR_LABEL) std::cout << "\n";
					if (it.second == ir.opcode) {
						std::cout << std::setw(14) << std::left << it.first
									<< std::setw(11) << ir.dst
									<< std::setw(11) << ir.src
									<< std::setw(11) << ir.size << std::endl;

						blFound = true;
						break;
					}
				}
				if (!blFound) _D(LOG_CRIT, "Cannot found opcode #%d", ir.opcode);
			} else {
				IR::emit(ir.opcode, ir.dst, ir.src, ir.size);
			}
		}

		this->_ir_stack_.clear();
		IR::localvar_reset();
	}
}
