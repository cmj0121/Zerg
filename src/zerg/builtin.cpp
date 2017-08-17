/* Copyright (C) 2014-2017 cmj. All right reserved. */

#include "zerg.h"

AST *Zerg::builtin_syscall(AST *node) {
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
AST *Zerg::builtin_exit(AST *node) {
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

AST *Zerg::builtin_buffer(AST *node) {
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
	this->emit(IR_MEMORY_STORE, node->data(), sub->data(), ZASM_MEM_QWORD, "0x0");
	this->emit(IR_ARITHMETIC_ADD, node->data(), "0x08");

	return node;
}
