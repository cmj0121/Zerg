/* Copyright (C) 2014-2017 cmj. All right reserved. */

#include <iostream>
#include <iomanip>
#include <set>

#include <unistd.h>
#include "zerg.h"


void Zerg::compile(std::string src) {
	AST *node = NULL;

	_D(LOG_INFO, "compile zerg with dummy loader");
	/* Set the entry point of the program */
	this->emit(IR_LABEL, ZASM_ENTRY_POINT);
	this->emit(IR_CONDITION_CALL, __IR_REFERENCE__ ZASM_MAIN_FUNCTION);
	this->emit(IR_MEMORY_PUSH, "0x2000001");
	this->emit(IR_INTERRUPT);


	/* compile the AST-CFG */
	_D(LOG_INFO, "compile source code and parse as - AST %s", src.c_str());
	node = Parser::parser(src);
	this->emitIR_stmt(node, true);

	_D(LOG_INFO, "compile sub-routine #%lu", this->_subroutine_.size());
	for (auto it : this->_subroutine_) {
		/* compile sub-routine */
		this->emitIR_stmt(it, true);
	}

	/* dump all globals symbol */
	_D(LOG_INFO, "dump all global symbol");
	this->emit(IR_LABEL, "__GLOBALS__");
	this->globals_str.push_back(std::make_pair(ZERG_TRUE,  "\"" ZERG_TRUE "\""));
	this->globals_str.push_back(std::make_pair(ZERG_FALSE, "\"" ZERG_FALSE "\""));
	for (auto it : this->globals_str) {
		/* FIXME - Should we need globals defined? of just set in local part */
		this->emit(IR_DEFINE, it.first, it.second);
	}

	_D(LOG_INFO, "end compile %s", src.c_str());
	IR::dump();
}

void Zerg::emit(AST *node, bool init) {
	if (init) {
		this->emit(IR_PROLOGUE);
		switch(node->type()) {
			case ZTYPE_CMD_FUNCTION:
			case ZTYPE_CMD_CLASS:
				break;
			default:
				this->emit(IR_LABEL, node->label());
				break;
		}
	}

	for (AST *cur = node; NULL != cur; cur = cur->transfer()) {
		std::string label;
		AST *sub = NULL, *tmp = NULL;

		#if defined(DEBUG_CFG) || defined(DEBUG)
			std::cerr << "==== CFG node " << node->label() << " ====" << std::endl;
			std::cerr << *cur << std::endl;
		#endif /* DEBUG_CFG */

		if (CFG_BRANCH == cur->ctype() && cur == cur->branch(false)) {
			sub = cur->branch(true);
			this->emit(IR_LABEL, sub->label());
		}
		this->emitIR(cur);

		switch(cur->ctype()) {
			case CFG_BRANCH:
				sub = cur->branch(true);
				tmp = cur->branch(false);

				if (NULL == cur->branch(false)) {
					label = __IR_REFERENCE__ + sub->label();
					this->emit(IR_CONDITION_JMPIFN, label, cur->data());

					this->emit(sub);
					this->emit(IR_LABEL, sub->label());
				} else if (cur == cur->branch(false)) {     /* LOOP */
					label = __IR_REFERENCE__ + sub->label(false);
					this->emit(IR_CONDITION_JMPIFN, label, cur->data());

					this->_loop_label_.push_back(__IR_REFERENCE__ + sub->label());
					this->emit(sub);
					this->_loop_label_.pop_back();

					this->emit(IR_CONDITION_JMP, __IR_REFERENCE__ + sub->label());
					this->emit(IR_LABEL, sub->label(false));
				} else {
					label = __IR_REFERENCE__ + sub->label();
					this->emit(IR_CONDITION_JMPIFN, label, cur->data());

					this->emit(sub);
					this->emit(IR_CONDITION_JMP, __IR_REFERENCE__ + tmp->label(false));

					this->emit(IR_LABEL, sub->label());
					this->emit(tmp);
					this->emit(IR_LABEL, tmp->label(false));
				}
				break;
			case CFG_BLOCK:
				/* NOP */
				break;
			default:
				if (NULL != cur->transfer())
					_D(LOG_CRIT, "Not Implemented #%d", cur->ctype());
				break;
		}
	}

	if (init) {
		/* PROLOGUE and EPILOGUE */
		this->emit(IR_EPILOGUE);
		this->emit(IR_CONDITION_RET);
	}
}
void Zerg::emit(IROP opcode, STRING dst, STRING src, STRING size, STRING idx) {
	IRInstruction ir = {opcode, dst, src, size, idx, -1};

	(*this) += ir;
}
