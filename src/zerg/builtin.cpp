/* Copyright (C) 2014-2017 cmj. All right reserved. */

#include "zerg.h"

AST* Zerg::builtin_syscall(AST *node) {
	AST *sub = NULL;

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

	return node;
}
AST* Zerg::builtin_exit(AST *node) {
	AST *sub = NULL;

	ALERT(2 != node->length() || 1 != (sub = node->child(1))->length());
	sub = sub->child(0);
#if __APPLE__ && __x86_64__
	this->emit(IR_MEMORY_PUSH, "0x2000001");
#else
	# error "Not Implemented"
#endif /**/
	this->emitIR(sub);
	this->emit(IR_MEMORY_PUSH, sub->data());
	this->emit(IR_INTERRUPT);

	return node;
}

AST* Zerg::builtin_buffer(AST *node) {
	AST *sub = NULL;

	ALERT(2 != node->length() || 1 != (sub = node->child(1))->length());
	sub = sub->child(0);

#if __APPLE__ && __x86_64__
	this->emit(IR_MEMORY_PUSH, "0x20000C5");
	this->emit(IR_MEMORY_PUSH, "0x0");
	this->emitIR(sub);
	this->emit(IR_ARITHMETIC_ADD, sub->data(), "0x08");
	this->emit(IR_MEMORY_PUSH, sub->data());
	this->emit(IR_MEMORY_PUSH, "0x7");
	this->emit(IR_MEMORY_PUSH, "0x1001");
	this->emit(IR_MEMORY_PUSH, "0xFFFFFFFF");
	this->emit(IR_MEMORY_PUSH, "0x0");

#else
	# error "Not Implemented"
#endif /**/

	this->emit(IR_INTERRUPT);
	node->setReg(-1);	/* __IR_SYSCALL_REG__ */
	/* FIXME - Should we re-compile again? */
	this->emitIR(sub);
	this->emit(IR_MEMORY_STORE, node->data(), sub->data(), ZASM_MEM_QWORD, "0x0");
	this->emit(IR_ARITHMETIC_ADD, node->data(), "0x08");

	node->otype(OBJ_BUFFER);
	return node;
}

AST* Zerg::builtin_delete(AST *node) {
	switch(_obj_type_map_[node->raw()]) {
		case OBJ_BUFFER:
			node->setReg(++ this->_regcnt_);

			this->emit(IR_MEMORY_PUSH, "0x2000049");
			this->emit(IR_MEMORY_LOAD, node->data(), node->raw());
			this->emit(IR_ARITHMETIC_SUB, node->data(), "0x08");
			this->emit(IR_MEMORY_PUSH, node->data());

			this->emit(IR_MEMORY_LOAD, node->data(), node->data(), ZASM_MEM_QWORD, "0x0");
			this->emit(IR_ARITHMETIC_ADD, node->data(), "0x08");
			this->emit(IR_MEMORY_PUSH, node->data());
			this->emit(IR_INTERRUPT);
			break;
		case OBJ_OBJECT:
		default:
			_D(LOG_CRIT, "Not Implemented delete %s #%d",
								node->raw().c_str(), _obj_type_map_[node->raw()]);
			break;
	}

	return node;
}

AST* Zerg::builtin_print(AST *node) {
	char buff[BUFSIZ] = {0};
	std::string label;

	switch(node->type()) {
		case ZTYPE_TRUE:
			node->setReg(++ this->_regcnt_);
			this->emit(IR_MEMORY_PUSH, "0x2000004");
			this->emit(IR_MEMORY_PUSH, "0x01");
			this->emit(IR_MEMORY_STORE, node->data(), __IR_REFERENCE__ ZERG_TRUE);
			this->emit(IR_MEMORY_PUSH, node->data());
			this->emit(IR_MEMORY_PUSH, "0x04");
			this->emit(IR_INTERRUPT);
			break;
		case ZTYPE_FALSE:
			node->setReg(++ this->_regcnt_);
			this->emit(IR_MEMORY_PUSH, "0x2000004");
			this->emit(IR_MEMORY_PUSH, "0x01");
			this->emit(IR_MEMORY_STORE, node->data(), __IR_REFERENCE__ ZERG_FALSE);
			this->emit(IR_MEMORY_PUSH, node->data());
			this->emit(IR_MEMORY_PUSH, "0x05");
			this->emit(IR_INTERRUPT);
			break;
		case ZTYPE_IDENTIFIER:
			switch(_obj_type_map_[node->raw()]) {
				case OBJ_BUFFER:
					this->emit(IR_MEMORY_PUSH, "0x2000004");
					this->emit(IR_MEMORY_PUSH, "0x01");
					this->emit(IR_MEMORY_PUSH, node->data());
					this->emit(IR_MEMORY_PUSH, "0x05");	/* FIXME - call strlen */
					this->emit(IR_INTERRUPT);
					break;
				default:
					_D(LOG_CRIT, "Not Implemented %s #%d", node->raw().c_str(),
															_obj_type_map_[node->raw()]);
					break;
			}
			break;
		case ZTYPE_STRING:
			//label = IR::randstr(16, "_str_");
			label = "_abcdefg";
			snprintf(buff, sizeof(buff), "0x%zX", node->raw().size()-2);

			node->setReg(++ this->_regcnt_);
			this->globals_str.push_back(std::make_pair(label, node->raw()));
			this->emit(IR_MEMORY_PUSH, "0x2000004");
			this->emit(IR_MEMORY_PUSH, "0x01");
			this->emit(IR_MEMORY_STORE, node->data(), __IR_REFERENCE__ + label);
			this->emit(IR_MEMORY_PUSH, node->data());
			this->emit(IR_MEMORY_PUSH, buff);
			this->emit(IR_INTERRUPT);
			break;
		default:
			_D(LOG_CRIT, "Not Implemented %s #%d", node->raw().c_str(), node->type());
			break;
	}

	return node;
}
