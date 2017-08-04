/* Copyright (C) 2014-2017 cmj. All right reserved. */

#include <iostream>
#include <iomanip>
#include <set>

#include <unistd.h>
#include "zerg.h"


void Zerg::compile(std::string src) {
	AST *node = NULL;
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

	/* compile the AST-CFG */
	node = Parser::parser(src);
	node = this->parser(node);
	this->emit(node, true);

	for (auto it : this->_subroutine_) {
		/* compile sub-routine */
		this->emit(it, true);
	}

	/* dump all globals symbol */
	this->emit(IR_LABEL, "__GLOBALS__");
	for (auto it : this->globals_str) {
		/* FIXME - Should we need globals defined? of just set in local part */
		this->emit(IR_DEFINE, it.first, it.second);
	}
	this->flush();
}

AST *Zerg::parser(AST *node) {
	int cnt = 0;
	AST *cur = NULL, *sub = NULL;

	if (NULL == node) return node;

	_D(LOG_DEBUG, "parse AST node %s", node->raw().c_str());
	/* Split into several CFG */
	switch(node->type()) {
		case ZTYPE_UNKNOWN:
			while (cnt < node->length()) {
				switch(node->child(cnt)->type()) {
					case ZTYPE_CMD_FUNCTION:
					case ZTYPE_CMD_CLASS:
						cur = node->child(cnt);
						this->parser(cur);
						break;
					case ZTYPE_CMD_IF:
						/* remainder- part */
						if (cnt+1 < node->length()) {
							/* split the next CFG stage */
							sub = node->split(cnt+1);
						}
						/* condition */
						cur = node->child(cnt);
						cur = this->parser(cur);
						node->transfer(cur);
						if (NULL != sub) {
							sub = this->parser(sub);
							node->transfer(sub);
						}
						break;
					default:
						++cnt;
						break;
				}
			}
			break;
		case ZTYPE_CMD_IF:
			node->remove();
			cur = node->child(1);
			sub = 3 == node->length() ? node->child(2) : NULL;

			cur->remove();
			if (NULL != sub) sub->remove();
			cur = this->parser(cur);
			sub = this->parser(sub);

			node = node->branch(cur, sub);
			break;
		case ZTYPE_CMD_FUNCTION:
		case ZTYPE_CMD_CLASS:
			/* exactly sub-routine */
			node->remove();
			this->_subroutine_.push_back(node);
			break;
		default:
			_D(LOG_CRIT, "Not Implemented #%d", node->type());
			break;
	}

	return node;
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

	if (this->_ir_stack_.size()) {

		_D(LOG_DEBUG_IR, "flush #%lu IR", this->_ir_stack_.size());
		for (auto ir : this->_ir_stack_) {
			if (this->_args_.only_ir) {
				for (auto it : IROP_map) {
					if (it.second == ir.opcode) {
						if (it.second == IR_LABEL) std::cout << "\n";
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
