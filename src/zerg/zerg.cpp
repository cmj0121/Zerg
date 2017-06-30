/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iostream>
#include <iomanip>

#include <unistd.h>
#include "zerg.h"


void Zerg::compile(std::string src) {
	if (this->_args_.compile_ir) {
		IR::compile(src);
	} else {
		/* load the built-in library if possible */
		if (0 == access(BUILTIN_LIBRARY, F_OK) && false == this->_args_.no_stdlib) {
			_D(LOG_INFO, "Load the built-in library `%s`", BUILTIN_LIBRARY);
			//this->parser(BUILTIN_LIBRARY);
		}
		this->parser(src);
	}
}

AST* Zerg::parser(std::string srcfile) {
	AST *node = Parser::parser(srcfile);

	/* generate the CFG is possible */

	/* emit the IR from CFG-AST */
	this->emitIR(node);
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
AST* Zerg::emitIR(AST *node) {
	AST *sub = NULL;

	ALERT(NULL == node);

	_D(LOG_DEBUG_IR, "emit IR on %s #%d", node->raw().c_str(), node->type());
	switch(node->type()) {
		case ZTYPE_UNKNOWN:
			for (ssize_t i = 0; i < node->length(); ++i) {
				this->emitIR(node->child(i));
			}
			break;

		/* atom */
		case ZTYPE_NUMBER:
		case ZTYPE_STRING:
		case ZTYPE_IDENTIFIER:
			this->emitIR_atom(node);
			break;

		/* arithmetic */
		case ZTYPE_INC: case ZTYPE_DEC:
		case ZTYPE_ADD: case ZTYPE_SUB: case ZTYPE_LIKE: case ZTYPE_LOG_NOT:
			if (1 == node->length()) {
				this->emitIR_atom(node);
				break;
			}

		case ZTYPE_MUL: case ZTYPE_DIV: case ZTYPE_MOD: case ZTYPE_POW:
		case ZTYPE_RSHT: case ZTYPE_LSHT:
		case ZTYPE_BIT_AND: case ZTYPE_BIT_OR: case ZTYPE_BIT_XOR:
		case ZTYPE_LOG_AND: case ZTYPE_LOG_OR: case ZTYPE_LOG_XOR:
		case ZTYPE_CMP_EQ: case ZTYPE_CMP_LS: case ZTYPE_CMP_GT:
			this->emitIR_arithmetic(node);
			break;

		/* assignment */
		case ZTYPE_LASSIGN: case ZTYPE_RASSIGN:
			this->emitIR_assignment(node);
			break;

		case ZTYPE_FUNCCALL:
		case ZTYPE_GETTER:
		case ZTYPE_CMD_IN:

		case ZTYPE_CMD_INCLUDE:
		case ZTYPE_CMD_PRINT:
			_D(LOG_CRIT, "Not implemented on %s #%d", node->raw().c_str(), node->type());
			break;
		/* sub-routine */
		case ZTYPE_CMD_FUNCTION: case ZTYPE_CMD_CLASS:
		case ZTYPE_CMD_WHILE: case ZTYPE_CMD_FOR:
		case ZTYPE_CMD_IF:
			this->emitIR_subroutine(node);
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
AST* Zerg::emitIR_atom(AST *node) {
	AST *x = NULL;

	_D(LOG_DEBUG_IR, "emit IR on atom %s", node->raw().c_str());
	switch(node->type()) {
		/* term */
		case ZTYPE_ADD:
			ALERT(1 != node->length());
			x = this->emitIR_atom(node->child(0));
			node->setReg(x->getReg());
			break;
		case ZTYPE_SUB:
			ALERT(1 != node->length());
			x = this->emitIR_atom(node->child(0));
			this->emit(IR_LOGICAL_NEG, x->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_LOG_NOT:
			ALERT(1 != node->length());
			x = this->emitIR_atom(node->child(0));
			this->emit(IR_LOGICAL_NEG, x->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_INC:
			ALERT(1 != node->length());
			x = this->emitIR_atom(node->child(0));
			this->emit(IR_ARITHMETIC_INC, x->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_DEC:
			ALERT(1 != node->length());
			x = this->emitIR_atom(node->child(0));
			this->emit(IR_ARITHMETIC_DEC, x->data());
			node->setReg(x->getReg());
			break;

		/* atom */
		case ZTYPE_NUMBER:
			node->setReg(++this->_regcnt_);
			this->emit(IR_MEMORY_STORE, node->data(), node->raw());
			break;
		case ZTYPE_STRING:
		case ZTYPE_IDENTIFIER:
		default:
			_D(LOG_CRIT, "Not implemented on %s #%d", node->raw().c_str(), node->type());
			break;
	}

	return node;
}
AST* Zerg::emitIR_arithmetic(AST *node) {
	AST *x = NULL, *y = NULL;

	ALERT(2 != node->length());

	x = this->emitIR(node->child(0));
	y = this->emitIR(node->child(1));
	_D(LOG_DEBUG_IR, "emit IR on arithmetic %s", node->raw().c_str());
	switch(node->type()) {
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

		/* bitwise */
		case ZTYPE_BIT_AND:
		case ZTYPE_BIT_OR:
		case ZTYPE_BIT_XOR:
			_D(LOG_CRIT, "Not implemented on %s #%d", node->raw().c_str(), node->type());
			break;

		/* logical */
		case ZTYPE_LOG_AND:
			this->emit(IR_LOGICAL_AND, x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_LOG_OR:
			this->emit(IR_LOGICAL_OR, x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_LOG_XOR:
			this->emit(IR_LOGICAL_XOR, x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_LOG_NOT:
			this->emit(IR_LOGICAL_NOT, x->data());
			node->setReg(x->getReg());
			break;

		/* compare */
		case ZTYPE_CMP_EQ:
			this->emit(IR_LOGICAL_EQ, x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_CMP_LS:
			this->emit(IR_LOGICAL_LS, x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_CMP_GT:
			this->emit(IR_LOGICAL_GT, x->data(), y->data());
			node->setReg(x->getReg());
			break;

		default:
			_D(LOG_CRIT, "Not implemented on %s #%d", node->raw().c_str(), node->type());
			break;
	}

	return node;
}
AST* Zerg::emitIR_assignment(AST *node) {
	AST *dst = NULL, *expr = NULL;

	ALERT(2 != node->length());
	_D(LOG_DEBUG_IR, "emit IR on assignment %s", node->raw().c_str());
	switch(node->type()) {
		case ZTYPE_LASSIGN:
			dst  = node->child(0);
			expr = this->emitIR(node->child(1));
			break;
		case ZTYPE_RASSIGN:
			dst  = node->child(1);
			expr = this->emitIR(node->child(0));
			break;
		default:
			_D(LOG_CRIT, "Not implemented on %s #%d", node->raw().c_str(), node->type());
			break;
	}

	if (0 != dst->length()) {
		_D(LOG_CRIT, "Not Implemented");
	}

	this->emit(IR_MEMORY_STORE, __IR_REFERENCE__ + dst->raw(), expr->data());
	return node;
}
AST* Zerg::emitIR_subroutine(AST *node) {
	AST *x = NULL, *y = NULL, *z = NULL;

	_D(LOG_DEBUG_IR, "emit IR on subroutine %s", node->raw().c_str());
	switch(node->type()) {
		case ZTYPE_CMD_IF:
			ALERT(2 > node->length());

			x  = node->child(0);
			y = node->child(1);
			if (3 == node->length()) z = node->child(2);

			x = this->emitIR(x);
			this->emit(IR_LOGICAL_NOT, x->data());
			this->emit(IR_CONDITION_JMPIF, __IR_REFERENCE__  "tmp_label", x->data());
			this->emitIR(y);
			if (NULL != z) this->emit(IR_CONDITION_JMP, "tmp_label_end");
			this->emit(IR_LABEL, "tmp_label");

			if (NULL != z) {
				this->emitIR(z);
				this->emit(IR_LABEL, "tmp_label_end");
			}
			break;
		case ZTYPE_CMD_WHILE:
			ALERT(2 != node->length());

			x  = node->child(0);
			y = node->child(1);

			this->emit(IR_LABEL, "tmp_label_start");
			x = this->emitIR(x);
			this->emit(IR_CONDITION_JMPIF, __IR_REFERENCE__  "tmp_label", x->data());
			this->emitIR(y);
			this->emit(IR_CONDITION_JMP, __IR_REFERENCE__ "tmp_label_start");
			this->emit(IR_LABEL, "tmp_label");
			break;

		case ZTYPE_CMD_FUNCTION:
			ALERT(3 != node->length());

			x = node->child(0);
			y = node->child(1);
			z = node->child(2);

			this->emit(IR_LABEL, x->raw());
			/* FIXME - parameter */
			this->emitIR(z);
			break;
		case ZTYPE_CMD_CLASS:
			ALERT(3 != node->length());

			x = node->child(0);
			y = node->child(1);
			z = node->child(2);

			this->emit(IR_LABEL, x->raw());
			/* FIXME - inherit */
			this->emitIR(z);
			break;

		default:
			_D(LOG_CRIT, "Not implemented on %s #%d", node->raw().c_str(), node->type());
			break;
	}

	return node;
}
