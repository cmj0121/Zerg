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
	this->flush();


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
	for (auto it : this->globals_str) {
		/* FIXME - Should we need globals defined? of just set in local part */
		this->emit(IR_DEFINE, it.first, it.second);
	}
	this->flush();

	_D(LOG_INFO, "end compile %s", src.c_str());
}

void Zerg::emit(AST *node, bool init) {
	if (init) {
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
		char buff[BUFSIZ] = {0};
		ZergIR prologue;

		snprintf(buff, sizeof(buff), "0x%lX", IR::localvar_len() * PARAM_SIZE);
		prologue.opcode = IR_PROLOGUE;
		prologue.dst    = buff;

		/* PROLOGUE and EPILOGUE */
		this->_ir_stack_.insert(this->_ir_stack_.begin()+1, prologue);
		this->emit(IR_EPILOGUE, buff);
		this->emit(IR_CONDITION_RET);
		this->flush();
	}
}
void Zerg::emit(IROP opcode, STRING dst, STRING src, STRING size, STRING idx) {
	ZergIR ir = {opcode, dst, src, size, idx};

	/* count the local variable */
	if (opcode == IR_MEMORY_STORE && IR_TOKEN_VAR == IR::token(dst)) {
		IR::localvar(dst);
	}
	this->_ir_stack_.push_back(ir);
}
void Zerg::flush(void) {
	bool blFound = false;
	char buff[BUFSIZ] = {0};

	if (this->_ir_stack_.size()) {
		snprintf(buff, sizeof(buff), "0x%lX", IR::localvar_len() * PARAM_SIZE);
		_D(LOG_DEBUG_IR, "flush #%lu IR", this->_ir_stack_.size());
		for (auto ir : this->_ir_stack_) {
			if (IR_EPILOGUE == ir.opcode && "" == ir.dst) ir.dst = buff;

			if (this->_args_.only_ir) {
				for (auto it : IROP_map) {
					if (it.second == ir.opcode) {
						if (it.second == IR_LABEL) std::cout << "\n";
						std::cout << std::setw(14) << std::left << it.first;
						if ("" != ir.dst)	std::cout << std::setw(24) << ir.dst;
						if ("" != ir.src)	std::cout << std::setw(24) << ir.src;
						if ("" != ir.size)	std::cout << std::setw(24) << ir.size;
						if ("" != ir.index) std::cout << std::setw(24) << ir.index;
						std::cout << std::endl;

						blFound = true;
						break;
					}
				}
				if (!blFound) _D(LOG_CRIT, "Cannot found opcode #%d", ir.opcode);
			} else {
				IR::emit(ir.opcode, ir.dst, ir.src, ir.size, ir.index);
			}
		}

		this->_ir_stack_.clear();
		IR::localvar_reset();
	}
}
