/* Copyright (C) 2014-2017 cmj. All right reserved. */

#include "zerg.h"

AST* Zerg::emitIR(AST *node) {
	std::string tmp;
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
		case ZTYPE_NONE: case ZTYPE_TRUE: case ZTYPE_FALSE:
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
			if (ZTYPE_BUILTIN_SYSCALL == node->child(0)->type()) {
				ALERT(2 != node->length() || 0 == node->child(1)->length());

				for (int i = 0; i < node->child(1)->length(); ++i) {
					sub = node->child(1)->child(i);

					switch(sub->type()) {
						case ZTYPE_NUMBER:
						case ZTYPE_IDENTIFIER:
							this->emit(IR_MEMORY_PUSH, sub->raw());
							break;
						default:
							this->emitIR(sub);
							this->emit(IR_MEMORY_PUSH, sub->data());
							break;
					}
				}
				this->emit(IR_INTERRUPT);
			} else {
				int cnt = 0;
				char buff[BUFSIZ] = {0};

				/* general function call */
				if (1 < node->length()) {
					sub = node->child(1);
					switch(sub->type()) {
						case ZTYPE_COMMA:
						default:
							this->emitIR(sub);
							this->emit(IR_MEMORY_PUSH, sub->data());
							++cnt;
							break;
					}
				}

				sub = node->child(0);
				this->emit(IR_CONDITION_CALL, __IR_REFERENCE__ + sub->data());
				snprintf(buff, sizeof(buff), __IR_REG_FMT__, ++this->_regcnt_);
				while (0 < cnt) {
					this->emit(IR_MEMORY_POP, buff);
					--cnt;
				}
			}

			break;
		case ZTYPE_GETTER:
		case ZTYPE_CMD_IN:

		case ZTYPE_CMD_INCLUDE:
		case ZTYPE_CMD_PRINT:
			_D(LOG_CRIT, "Not implemented on %s #%d", node->raw().c_str(), node->type());
			break;
		/* sub-routine */
		case ZTYPE_CMD_FUNCTION: case ZTYPE_CMD_CLASS:
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
	std::pair<std::string, std::string> symb;

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
		case ZTYPE_NONE:
			node->otype(OBJ_NONE);
			break;
		case ZTYPE_TRUE: case ZTYPE_FALSE:
			node->otype(OBJ_BOOLEAN);
			break;
		case ZTYPE_NUMBER:
			node->otype(OBJ_INT);
			node->setReg(++this->_regcnt_);
			this->emit(IR_MEMORY_STORE, node->data(), node->raw());
			break;
		case ZTYPE_STRING:
			node->otype(OBJ_STRING);
			symb = std::make_pair(randstr(8), node->raw());
			globals_str.push_back(symb);
			node->setSymb(symb.first);
			break;
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
			this->emit(IR_LOGICAL_AND, x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_BIT_OR:
			this->emit(IR_LOGICAL_OR, x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_BIT_XOR:
			this->emit(IR_LOGICAL_XOR, x->data(), y->data());
			node->setReg(x->getReg());
			break;

		/* logical */
		case ZTYPE_LOG_AND:
			_D(LOG_CRIT, "Not Implemented");
			node->otype(OBJ_BOOLEAN);
			break;
		case ZTYPE_LOG_OR:
			_D(LOG_CRIT, "Not Implemented");
			node->otype(OBJ_BOOLEAN);
			break;
		case ZTYPE_LOG_XOR:
			_D(LOG_CRIT, "Not Implemented");
			node->otype(OBJ_BOOLEAN);
			break;
		case ZTYPE_LOG_NOT:
			_D(LOG_CRIT, "Not Implemented");
			node->otype(OBJ_BOOLEAN);
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
	std::string tmp;
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

	switch(expr->otype()) {
		case OBJ_NONE:
			this->emit(IR_MEMORY_STORE, dst->raw(), "0x0");
			break;
		case OBJ_BOOLEAN:
			tmp = ZTYPE_TRUE == expr->type() ? "0x1" : "0x0";
			this->emit(IR_MEMORY_STORE, dst->raw(), tmp);
			break;
		case OBJ_STRING:
			tmp = expr->data();
			expr->setReg(++this->_regcnt_);

			this->emit(IR_MEMORY_STORE, expr->data(), tmp);
			this->emit(IR_MEMORY_STORE, dst->raw(), expr->data());
			break;
		default:
			this->emit(IR_MEMORY_STORE, dst->raw(), expr->data());
			break;
	}

	node->otype(expr->otype());
	_D(LOG_DEBUG, "set object type %s -> 0x%X", node->raw().c_str(), node->otype());
	return node;
}
AST* Zerg::emitIR_subroutine(AST *node) {
	AST *x = NULL, *y = NULL, *z = NULL;

	_D(LOG_DEBUG_IR, "emit IR on subroutine %s", node->raw().c_str());
	switch(node->type()) {
		case ZTYPE_CMD_FUNCTION:
			ALERT(3 != node->length());

			x = node->child(0);
			y = 1 == node->child(1)->length() ? node->child(1)->child(0) : NULL;
			z = node->child(2);

			this->emit(IR_LABEL, x->raw());
			for (int i = 0; NULL != y && i < y->length(); ++i) {
				/* process the parameter */
				this->emit(IR_MEMORY_PARAM, y->child(i)->raw());
			}

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
