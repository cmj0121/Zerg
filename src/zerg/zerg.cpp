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
AST* Zerg::emitIR(AST *node) {
	AST *sub = NULL, *x = NULL, *y = NULL;
	ALERT(NULL == node);

	_D(LOG_DEBUG_IR, "emit IR on %s", node->raw().c_str());
	switch(node->type()) {
		/* arithmetic */
		case ZTYPE_ADD:
		case ZTYPE_SUB:
		case ZTYPE_MUL:
		case ZTYPE_DIV:
		case ZTYPE_MOD:
		case ZTYPE_LIKE:
		case ZTYPE_RSHT:
		case ZTYPE_LSHT:

		case ZTYPE_BIT_AND:
		case ZTYPE_BIT_OR:
		case ZTYPE_BIT_XOR:

		case ZTYPE_LOG_AND:
		case ZTYPE_LOG_OR:
		case ZTYPE_LOG_XOR:
		case ZTYPE_LOG_NOT:

		case ZTYPE_CMP_EQ:
		case ZTYPE_CMP_LS:
		case ZTYPE_CMP_GT:
			ALERT(2 != node->length());
			x = this->emitIR(node->child(0));
			y = this->emitIR(node->child(1));
			break;
		default:
			break;
	}

	switch(node->type()) {
		/* atom */
		case ZTYPE_NUMBER:
			node->setReg(++this->_regcnt_);
			this->emit(IR_MEMORY_STORE, node->data(), node->raw());
			break;
		case ZTYPE_STRING:
		case ZTYPE_IDENTIFIER:
			_D(LOG_CRIT, "Not implemented on %s #%d", node->raw().c_str(), node->type());
			break;

		/* arithmetic */
		case ZTYPE_ADD:
			this->emit(IR_ARITHMETIC_ADD, x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_SUB:
			this->emit(IR_ARITHMETIC_SUB, x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_MUL:
			this->emit(IR_ARITHMETIC_MUL, x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_DIV:
			this->emit(IR_ARITHMETIC_DIV, x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_MOD:
			this->emit(IR_ARITHMETIC_MOD, x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_LIKE:
		case ZTYPE_INC:
		case ZTYPE_DEC:
		case ZTYPE_POW:
			_D(LOG_CRIT, "Not implemented on %s #%d", node->raw().c_str(), node->type());
			break;
		case ZTYPE_RSHT:
			this->emit(IR_ARITHMETIC_SHR, x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_LSHT:
			this->emit(IR_ARITHMETIC_SHL, x->data(), y->data());
			node->setReg(x->getReg());
			break;

		case ZTYPE_BIT_AND:
		case ZTYPE_BIT_OR:
		case ZTYPE_BIT_XOR:

		case ZTYPE_LOG_AND:
		case ZTYPE_LOG_OR:
		case ZTYPE_LOG_XOR:
		case ZTYPE_LOG_NOT:

		case ZTYPE_CMP_EQ:
		case ZTYPE_CMP_LS:
		case ZTYPE_CMP_GT:

		case ZTYPE_FUNCCALL:
		case ZTYPE_GETTER:

		case ZTYPE_LASSIGN:
		case ZTYPE_RASSIGN:


		case ZTYPE_CMD_INCLUDE:
		case ZTYPE_CMD_PRINT:
		case ZTYPE_CMD_FUNCTION:
		case ZTYPE_CMD_CLASS:
		case ZTYPE_CMD_WHILE:
		case ZTYPE_CMD_FOR:
		case ZTYPE_CMD_IN:
		case ZTYPE_CMD_IF:
		case ZTYPE_CMD_CONTINUE:
		case ZTYPE_CMD_BREAK:
			_D(LOG_CRIT, "Not implemented on %s #%d", node->raw().c_str(), node->type());
			break;

		case ZTYPE_CMD_NOP:
			ALERT(0 != node->length());
			this->emit(IR_NOP);
			break;
		case ZTYPE_CMD_ASM:
			sub = node->child(0);
			ALERT(ZTYPE_STRING != sub->type() || 0 != sub->length());
			this->emit(IR_INLINE_ASM, sub->raw(), "", "");
			break;

		default:
			_D(LOG_CRIT, "Not implemented on %s #%d", node->raw().c_str(), node->type());
			break;
	}

	return node;
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
