/* Copyright (C) 2014-2017 cmj. All right reserved. */

#include <iostream>
#include "zerg.h"

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
						cur->remove();
						this->_subroutine_.push_back(cur);
						break;
					case ZTYPE_CMD_IF:
					case ZTYPE_CMD_WHILE:
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
		case ZTYPE_CMD_WHILE:
			node->remove();
			cur = node->child(1);

			cur->remove();
			cur = this->parser(cur);
			sub = this->parser(sub);

			node = node->branch(cur, node);
			break;
		case ZTYPE_CMD_FUNCTION:
		case ZTYPE_CMD_CLASS:
			/* NOP */
			break;
		default:
			_D(LOG_CRIT, "Not Implemented %s #%d", node->raw().c_str(), node->type());
			break;
	}

	#if defined(DEBUG_CFG) || defined(DEBUG)
		std::cerr << "parse AST node - " << node->raw() << std::endl;
		std::cerr << *node << std::endl;
	#endif /* DEBUG_CFG */
	return node;
}
AST* Zerg::emitIR(AST *node) {
	int cnt = 0;
	char buff[BUFSIZ] = {0};
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
		case ZTYPE_GETTER:
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

		/* change control flow */
		case ZTYPE_CMD_BREAK:
			if (0 == _loop_label_.size()) {
				_D(LOG_CRIT, "syntax error %s", node->data().c_str());
			}

			tmp = this->_loop_label_[this->_loop_label_.size()-1] + __IR_BRANCH_E__;
			this->emit(IR_CONDITION_JMP, tmp);
			break;
		case ZTYPE_CMD_CONTINUE:
			if (0 == _loop_label_.size()) {
				_D(LOG_CRIT, "syntax error %s", node->data().c_str());
			}

			tmp = this->_loop_label_[this->_loop_label_.size()-1];
			this->emit(IR_CONDITION_JMP, tmp);
			break;

		/* assignment */
		case ZTYPE_LASSIGN: case ZTYPE_RASSIGN:
			this->emitIR_assignment(node);
			break;

		case ZTYPE_FUNCCALL:
			ALERT (2 < node->length());

			sub = node->child(0);
			switch(sub->type()) {
				case ZTYPE_BUILTIN_SYSCALL:
					this->builtin_syscall(node);
					break;
				case ZTYPE_BUILTIN_EXIT:
					this->builtin_exit(node);
					break;
				case ZTYPE_BUILTIN_BUFFER:
					this->builtin_buffer(node);
					break;
				default:
					/* parameter */
					if (1 < node->length()) {
						_D(LOG_DEBUG, "function call %s with #%zd parameter",
									node->child(0)->data().c_str(), node->child(1)->length());
						for (int i = 0; i < node->child(1)->length(); ++i) {
							sub = node->child(1)->child(i);

							this->emitIR(sub);
							this->emit(IR_MEMORY_PUSH, sub->data());
							++cnt;
						}
					}

					sub = node->child(0);
					this->emit(IR_CONDITION_CALL, __IR_REFERENCE__ + sub->data());
					snprintf(buff, sizeof(buff), __IR_REG_FMT__, ++this->_regcnt_);
					while (0 < cnt) {
						this->emit(IR_MEMORY_POP, buff);
						--cnt;
					}
					break;
			}

			break;
		case ZTYPE_CMD_IN:

		case ZTYPE_CMD_INCLUDE:
		case ZTYPE_CMD_PRINT:
			_D(LOG_CRIT, "Not implemented on %s #%d", node->raw().c_str(), node->type());
			break;
		/* sub-routine */
		case ZTYPE_CMD_FUNCTION: case ZTYPE_CMD_CLASS:
			this->emitIR_subroutine(node);
			break;
		case ZTYPE_CMD_RETURN:
			ALERT (1 < node->length());

			for (sub = node->parent(); NULL != sub; sub = sub->parent()) {
				switch(sub->type()) {
					case ZTYPE_UNKNOWN:
						/* valid */
						break;
					case ZTYPE_CMD_FUNCTION:
						sub = sub->root();
						break;
					default:
						_D(LOG_CRIT, "Syntax error - %s", node->raw().c_str());
						break;
				}
			}

			if (0 != node->length()) {
				this->emitIR(node->child(0));
				this->emit(IR_MEMORY_STORE, __IR_SYSCALL_REG__, node->child(0)->data());
			}
			this->emit(IR_EPILOGUE);
			this->emit(IR_CONDITION_RET);
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

		case ZTYPE_CMD_DELETE:
			ALERT(1 != node->length() || 0 != (sub = node->child(0))->length());

			if (_obj_type_map_.end() == _obj_type_map_.find(sub->raw())) {
				_D(LOG_CRIT, "Variable %s not define", sub->raw().c_str());
			}

			this->builtin_delete(sub);
			break;

		default:
			_D(LOG_CRIT, "Not implemented on %s #%d", node->raw().c_str(), node->type());
			break;
	}

	return node;
}
AST* Zerg::emitIR_stmt(AST *ast, bool init) {
	/* parser the AST and emit the code  */
	AST *node = this->parser(ast);

	this->emit(node, init);
	return node;
}
AST* Zerg::emitIR_atom(AST *node) {
	AST *x = NULL, *y = NULL;
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
			x = this->emitIR(node->child(0));
			this->emit(IR_LOGICAL_EQ, x->data(), "0x0");
			node->setReg(x->getReg());
			break;
		case ZTYPE_INC:
			ALERT(1 != node->length());
			x = this->emitIR_atom(node->child(0));
			this->emit(IR_ARITHMETIC_INC, x->data());
            this->emit(IR_MEMORY_STORE, x->raw(), x->data());
			node->setReg(x->getReg());
			break;
		case ZTYPE_DEC:
			ALERT(1 != node->length());
			x = this->emitIR_atom(node->child(0));
			this->emit(IR_ARITHMETIC_DEC, x->data());
            this->emit(IR_MEMORY_STORE, x->raw(), x->data());
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
			if (NULL != node->parent() && ZTYPE_UNKNOWN != node->parent()->type()) {
				node->otype(OBJ_STRING);
				symb = std::make_pair(randstr(8), node->raw());
				globals_str.push_back(symb);
				node->setSymb(symb.first);
			}
			break;
		case ZTYPE_IDENTIFIER:
			node->setReg(++ this->_regcnt_);
			this->emit(IR_MEMORY_LOAD, node->data(), node->raw());
			break;

        /* atom - getter */
		case ZTYPE_GETTER:
            ALERT(2 != node->length() || ZTYPE_IDENTIFIER != node->child(0)->type());

            x = node->child(0);
            y = node->child(1);
            this->emitIR(y);
            node->setReg(++this->_regcnt_);
            this->emit(IR_MEMORY_LOAD, node->data(), x->data(), ZASM_MEM_QWORD, y->data());
            break;
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
	std::string tmp, index="", size=ZASM_MEM_QWORD;
	AST *dst = NULL, *expr = NULL, *idx = NULL;
	AST *x = NULL, *y = NULL;

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

	switch(dst->type()) {
		case ZTYPE_IDENTIFIER:
			/* general variable */
			if (0 != dst->length()) _D(LOG_CRIT, "Not Implemented %s", dst->raw().c_str());
			break;
		case ZTYPE_GETTER:
			ALERT(2 != dst->length());

			idx = dst->child(1);
			dst = dst->child(0);
			switch(idx->type()) {
				case ZTYPE_COLON:
					ALERT(2 != idx->length());

					x = idx->child(0);
					y = idx->child(1);
					if (ZTYPE_NUMBER != x->type() || ZTYPE_NUMBER != y->type()) {
						_D(LOG_CRIT, "Not Implemented");
					}

					switch(y->asInt() - x->asInt()) {
						case 1:
							size = ZASM_MEM_BYTE;
							break;
						case 2:
							size = ZASM_MEM_WORD;
							break;
						case 4:
							size = ZASM_MEM_DWORD;
							break;
						case 8:
							size = ZASM_MEM_QWORD;
							break;
						default:
							_D(LOG_CRIT, "Not Implemented 0x%llX", y->asInt()-x->asInt());
							break;
					}
					this->emitIR(x);
					index = x->data();
					break;
				default:
					this->emitIR(idx);
					size  = ZASM_MEM_BYTE;
					index = idx->data();
					break;
			}
			break;
		default:
			_D(LOG_CRIT, "Not Implemented for assign to object - %s", dst->raw().c_str());
			break;
	}

	switch(expr->otype()) {
		case OBJ_NONE:
			this->emit(IR_MEMORY_STORE, dst->raw(), "0x0", size, index);
			break;
		case OBJ_BOOLEAN:
			tmp = ZTYPE_TRUE == expr->type() ? "0x1" : "0x0";
			this->emit(IR_MEMORY_STORE, dst->raw(), tmp, size, index);
			break;
		case OBJ_STRING:
			tmp = expr->data();
			expr->setReg(++this->_regcnt_);

			this->emit(IR_MEMORY_STORE, expr->data(), tmp);
			this->emit(IR_MEMORY_STORE, dst->raw(), expr->data(), size, index);
			break;
		default:
			this->emit(IR_MEMORY_STORE, dst->raw(), expr->data(), size, index);
			break;
	}

	if (NULL == dst->parent() || ZTYPE_GETTER != dst->parent()->type()) {
		node->otype(expr->otype());
		this->_obj_type_map_[dst->raw()] = node->otype();
		_D(LOG_ERROR, "%s -> #%d", dst->raw().c_str(), node->otype());
	}

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
			if (NULL != y) {
				for (int i = y->length()-1; i >= 0; --i) {
					/* process the parameter */
					this->emit(IR_MEMORY_PARAM, y->child(i)->raw());
				}
			}

			/* parser the scope */
			z = this->parser(z);
			this->emit(z);
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
