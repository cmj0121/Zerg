/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <unistd.h>
#include <iomanip>
#include "zerg.h"

Zerg::Zerg(std::string dst, ZergArgs *args) : IR(dst, args) {
	this->_labelcnt_	= 0;
	this->_lineno_		= 1;
	this->_regs_		= 0;
}
Zerg::~Zerg() {
	/* delete the AST node */
	for (auto it : this->_root_) {
		delete it.second;
	}
}

void Zerg::compile(std::string src, ZergArgs *args) {
	if (args->_compile_ir_) {
		IR::compile(src);
	} else {
		this->_only_ir_ = args->_only_ir_;

		/* load the built-in library if possible */
		if (0 == access(BUILTIN_LIBRARY, F_OK) && false == args->_no_stdlib_) {
			_D(LOG_INFO, "Load the built-in library `%s`", BUILTIN_LIBRARY);
			this->lexer(BUILTIN_LIBRARY);
		}
		this->lexer(src);

		this->emit("#! /usr/bin/env zgr");
		this->emit("#! ZERG IR - v" __IR_VERSION__);
		this->emit("ASM", "call", "&" CFG_MAIN);
		this->emit("ASM", "mov", "rax", "0x2000001");
		this->emit("INTERRUPT");

		if (0 == this->_root_.count(CFG_MAIN)) {
			this->emit("LABEL", CFG_MAIN);
			this->emit("RET");
		}

		for (auto it : this->_root_) {
			/* compile subroutine */
			std::map<std::string, VType> namescope;
			_D(LOG_INFO, "compile subroutine `%s`", it.first.c_str());

			this->emit("# Sub-Routine - " + it.first);
			this->_stack_.clear();
			/* load the function status */
			this->load_namespace(namescope);
			this->compileCFG(it.second, namescope);
			this->emit("RET");
		}

		_D(LOG_INFO, "dump all symbols");
		this->_symb_.push_back(std::make_pair("TRUE",  "True\\n"));
		this->_symb_.push_back(std::make_pair("FALSE", "False\\n"));
		if (0 != this->_symb_.size()) {
			this->emit("# Dump all symbols");
			for (auto it : this->_symb_) {
				this->emit("LABEL", it.first, it.second);
			}
		}

		this->emit("# vim: set ft=zgr:");
	}
}
void Zerg::compileCFG(CFG *node, std::map<std::string, VType> &namescope) {
	if (NULL == node || node->isEmmited()) {
		_D(LOG_DEBUG, "already processed");
		return ;
	}

	this->emit("LABEL", node->label());
	if (0 == node->length()) {
		/* need NOT process this node */
		/* PROLOGUE */
		this->emit("PROLOGUE", node->varcnt());

		this->_compileCFG_(node->nextCFG(true), namescope);
		this->_compileCFG_(node->nextCFG(false), namescope);

		/* EPILOGUE */
		this->emit("EPILOGUE", node->varcnt());
	} else {
		/* PROLOGUE */
		this->emit("PROLOGUE", node->varcnt());

		this->_compileCFG_(node, namescope);

		/* EPILOGUE */
		this->emit("EPILOGUE", node->varcnt());
	}
}
void Zerg::_compileCFG_(CFG *node, std::map<std::string, VType> &namescope) {
	CFG *tmp = NULL;

	if (NULL == node) {
		return ;
	}

	_D(LOG_DEBUG, "compile CFG - %s", node->label().c_str());
	#ifdef DEBUG_CFG
	std::cerr << *node << std::endl;
	#endif /* DEBUG_CFG */
	/*
	 *         +---+
	 *         |   |           CONDITION NODE          test expression
	 *         +---+                                   JMP to [BRANCH FALSE] if false
	 *          / \
	 *         /   \
	 *     +---+  +---+                                statement for true branch
	 *     | T |  | F |        BRANCH NODE             , and then JMP to [NORMAL NODE]
	 *     +---+  +---+
	 *       \     /                                   statement for false branch
	 *         \ /
	 *        +---+
	 *        |   |            NORMAL NODE             statement for normal expression
	 *        +---+
	 */


	/* main logical to generate IR */
	if (NULL != node->prev() && node->prev()->isBranch()) {
		_D(LOG_DEBUG, "set label %s", node->label().c_str());
		this->emit("LABEL", node->label());
	} else if (node->isBranch() && (NULL == node->nextCFG(true) || node->nextCFG(false))) {
		_D(LOG_DEBUG, "set label %s", node->label().c_str());
		this->emit("LABEL", node->label());
	} else if (node->isCondit()) {
		_D(LOG_DEBUG, "set label %s", node->label().c_str());
		this->emit("LABEL", node->label());

		if (NULL != node->child(0) && AST_WHILE == node->child(0)->type()) {
			this->_repeate_label_.push_back(node->label());
		}
	}
	this->emitIR(node, namescope);

	if (node->isCondit()) {		/* CONDITION NODE */
		std::string label;

		if (NULL == node->nextCFG(false)) {
			label = node->label() + "_END";
		} else {
			label = node->nextCFG(false)->label();
		}
		this->emit("JMP_FALSE", label, node->data());
	} else if (node->isBranch() && NULL != (tmp = node->nextCFG(true))) {
		/* end of branch node, jump to next stage */
		this->emit("JMP", tmp->label());
	}

	/* process other stage in CFG */
	this->_compileCFG_(node->nextCFG(true), namescope);
	this->_compileCFG_(node->nextCFG(false), namescope);

	if (node->isBranch() && node == node->prev()->nextCFG(true)) {
		if (AST_WHILE == node->prev()->child(0)->type()) {
			_D(LOG_DEBUG, "Loop on the label %s", node->prev()->label().c_str());
			this->emit("JMP", node->prev()->label());
		}
	}

	if (node->isBranch()) {
		if (node == node->prev()->nextCFG(false) || NULL == node->prev()->nextCFG(false)) {
			_D(LOG_DEBUG, "set label %s_END", node->prev()->label().c_str());
			this->emit("LABEL", node->prev()->label() + "_END");
		}

		if (0 != node->length() && AST_WHILE == node->child(0)->type()) {
			this->_repeate_label_.pop_back();
		}
	}
}
void Zerg::emitIR(AST *node, std::map<std::string, VType> &namescope) {
	static std::vector<std::string> stack;

	char buf[BUFSIZ] = {0};
	std::string tmp;
	AST *x = NULL, *y = NULL, *cur = NULL;

	node->setEmitted();
	/* process first if need */
	switch(node->type()) {
		case AST_INC: case AST_DEC:
			if (1 != node->length() || AST_IDENTIFIER != node->child(0)->type()) {
				_D(LOG_CRIT, "Not allow the syntax for `%s`", AST_INC == node->type() ? "++" : "--");
				return ;
			}

			/* HACK - check the INC/DEC is the simple or not */
			switch(node->parent()->type()) {
				case AST_ROOT:
				case AST_PRINT:
					break;
				default:
					_D(LOG_CRIT, "INC/DEC only allow in simple statement");
					break;
			}

			x   = node->child(0);
			tmp = x->data();
			this->emitIR(x, namescope);
			this->emit(AST_INC == node->type() ? "INC" : "DEC", x->data());
			this->emit("STORE", tmp, x->data());
			node->setReg(x);
			return ;
		case AST_LOG_AND:
			ALERT(2 != node->length());

			tmp = IR::randstr();
			x = node->child(0);
			y = node->child(1);
			this->emitIR(x, namescope);
			this->emit("JMP_FALSE", tmp, x->data());
			this->emitIR(y, namescope);
			this->emit("STORE", x->data(), y->data(), x->getIndex(), x->getIndexSize());
			this->emit("LABEL", tmp);
			node->setReg(x);
			return;
		case AST_LOG_OR:
			ALERT(2 != node->length());

			tmp = IR::randstr();
			x = node->child(0);
			y = node->child(1);
			this->emitIR(x, namescope);
			this->emit("JMP_TRUE", tmp, x->data());
			this->emitIR(y, namescope);
			this->emit("STORE", x->data(), y->data(), x->getIndex(), x->getIndexSize());
			this->emit("LABEL", tmp);
			node->setReg(x);
			return;
		case AST_LOG_XOR:
			ALERT(2 != node->length());

			x = node->child(0);
			y = node->child(1);
			this->emitIR(x, namescope);
			this->emitIR(y, namescope);
			this->emit("XOR", x->data(), y->data());
			node->setReg(x);
			return ;
		case AST_LOG_NOT:
			ALERT(1 != node->length());

			x = node->child(0);
			this->emitIR(x, namescope);
			this->emit("XOR", x->data(), "0x1");
			node->setReg(x);
			return;
		case AST_BUILDIN_BUFFER:
			ALERT(1 != node->length());

			x = node->child(0);
			this->emitIR(x, namescope);
			node->setReg(x);
			return ;
		case AST_IDENTIFIER:
			if (2 == node->length()) {
				VType type = VTYPE_UNKNOWN;
				int idxsize = 1;

				switch(node->child(0)->type()) {
					case AST_PARENTHESES_OPEN:
						ALERT(0 == namescope.count(node->data()));
						node->vtype(namescope[node->data()]);

						x = node->child(0);
						for (size_t i = 0; i < x->length(); ++i) {
							this->emitIR(x->child(i), namescope);
							this->emit("PARAM", x->child(i)->data());
						}

						this->emit("CALL", node->data());
						node->setReg(SYSCALL_REG);

						/* HACK - Need not process anymore */
						return ;
					case AST_BRACKET_OPEN:
						ALERT(0 == node->child(0)->length() || 2 < node->child(0)->length());
						ALERT(0 == namescope.count(node->data()));

						x       = node->child(0)->child(0);
						type    = namescope[node->data()];

						if (2 == node->child(0)->length()) {
							idxsize  = atoi(node->child(0)->child(1)->data().c_str());
							idxsize -= atoi(x->data().c_str());
						}

						switch(type) {
							case VTYPE_BUFFER:
							case VTYPE_PARAM:
								this->emitIR(x, namescope);
								node->setIndex(x, idxsize);
								break;
							default:
								_D(LOG_CRIT, "`%s` need to be declare as __buffer__ 0x%X",
																	node->data().c_str(), type);
								break;
						}
						break;
					default:
						_D(LOG_CRIT, "Not Implemented");
						break;
				}
				break ;
			}

			for (size_t i = 0; i < node->length(); ++i) {
				this->emitIR(node->child(i), namescope);
			}
			break;
		case AST_SYSCALL:
			if (2 == node->length() && AST_PARENTHESES_OPEN == node->child(0)->type()) {
				for (size_t i = 0; i < node->child(0)->length(); ++i) {
					cur = node->child(0)->child(i);

					switch(cur->type()) {
						case AST_NUMBER:
							/* HACK - direct save into stack without save as register */
							this->emit("PARAM", cur->data());
							break;
						case AST_STRING:
							this->emitIR(cur, namescope);
							this->emit("PARAM", cur->data());
							break;
						default:
							this->emitIR(cur, namescope);
							this->emit("PARAM", cur->data());
							break;
					}
				}
				this->emit("INTERRUPT");
				node->setReg(SYSCALL_REG);
				return;
			}

			for (size_t i = 0; i < node->length(); ++i) {
				this->emitIR(node->child(i), namescope);
			}
			break;
		case AST_FUNC:
			cur = node->child(0);
			x   = cur->child(0);

			for (size_t i = 0; i < x->length(); ++i) {
				char idx[BUFSIZ] = {0};

				y = x->child(i);
				snprintf(idx, sizeof(idx), "[rbp+0x%zX]", (i+2) * 0x08);
				this->emit("STORE", y->data(), __IR_LOCAL_VAR__, idx);
				namescope[y->data()] = VTYPE_PARAM;
			}

			return ;
		default:
			/* Run DFS */
			for (size_t i = 0; i < node->length(); ++i) {
				this->emitIR(node->child(i), namescope);
			}
			break;
	}

	/* translate AST to IR */
	_D(LOG_DEBUG, "emit IR on %s #%zu", node->data().c_str(), namescope.size());
	switch(node->type()) {
		case AST_ROOT:
			/* NOP */
			if (0 != node->length()) {
				x = node->child(0);
				node->setReg(x);
			}
			break;
		case AST_TRUE:
			node->setReg(++this->_regs_);
			node->vtype(VTYPE_BOOLEAN);
			this->emit("STORE", node->data(), "1");
			break;
		case AST_FALSE:
			node->setReg(++this->_regs_);
			node->vtype(VTYPE_BOOLEAN);
			this->emit("STORE", node->data(), "0");
			break;
		case AST_NUMBER:
			tmp = node->data();
			node->setReg(++this->_regs_);
			this->emit("STORE", node->data(), tmp, node->getIndex(), node->getIndexSize());
			break;
		case AST_STRING:
			tmp = node->data();
			node->setLabel(++this->_labelcnt_);
			this->_symb_.push_back(std::make_pair(node->data(), tmp));
			break;
		case AST_IDENTIFIER:
			/* Load into register */
			switch(node->parent()->type()) {
				case AST_ASSIGN:
					/* need NOT load as variable */
					break;
				default:
					tmp = node->raw();
					node->vtype(namescope[node->data()]);
					node->setReg(++this->_regs_);

					this->emit("LOAD", node->data(), tmp, node->getIndex(), node->getIndexSize());
					break;
			}
			break;
		case AST_ADD:
			ALERT(2 < node->length() || 0 == node->length());

			switch(node->length()) {
				case 1:
					/* NOP */
					x = node->child(0);
					node->setReg(x);
					break;
					break;
				case 2:
					x = node->child(0);
					y = node->child(1);
					this->emit("ADD", x->data(), y->data());
					node->setReg(x);
					break;
				default:
					_D(LOG_CRIT, "Not Support ADD with #%zu", node->length());
					break;
			}
			break;
		case AST_SUB:
			ALERT(2 < node->length() || 0 == node->length());

			switch(node->length()) {
				case 1:
					x = node->child(0);
					if (VTYPE_BOOLEAN == x->vtype()) {
						this->emit("XOR", x->data(), "0x01");
					} else {
						this->emit("NEG", x->data());
					}
					node->setReg(x);
					break;
				case 2:
					x = node->child(0);
					y = node->child(1);
					this->emit("SUB", x->data(), y->data());
					node->setReg(x);
					break;
				default:
					_D(LOG_CRIT, "Not Support SUB with #%zu", node->length());
					break;
			}
			break;
		case AST_LIKE:
			ALERT(2 < node->length() || 0 == node->length());

			switch(node->length()) {
				case 1:
					x = node->child(0);
					if (VTYPE_BOOLEAN == x->vtype()) {
						this->emit("XOR", x->data(), "0x01");
					} else {
						this->emit("NOT", x->data());
					}
					node->setReg(x);
					break;
				default:
					_D(LOG_CRIT, "Not Support SUB with #%zu", node->length());
					break;
			}
			break;

		case AST_MUL:
			ALERT(2 != node->length())
			x = node->child(0);
			y = node->child(1);
			this->emit("MUL", x->data(), y->data());
			node->setReg(x);
			break;
		case AST_DIV:
			ALERT(2 != node->length())
			x = node->child(0);
			y = node->child(1);
			this->emit("DIV", x->data(), y->data());
			node->setReg(x);
			break;
		case AST_MOD:
			ALERT(2 != node->length())
			x = node->child(0);
			y = node->child(1);
			this->emit("REM", x->data(), y->data());
			node->setReg(x);
			break;

		case AST_LSHT:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("SHL", x->data(), y->data());
			node->setReg(x);
			break;
		case AST_RSHT:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("SHR", x->data(), y->data());
			node->setReg(x);
			break;

		case AST_LESS:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("LS", x->data(), y->data());
			node->setReg(x);
			break;
		case AST_LESS_OR_EQUAL:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("LE", x->data(), y->data());
			node->setReg(x);
			break;
		case AST_GRATE:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("GT", x->data(), y->data());
			node->setReg(x);
			break;
		case AST_GRATE_OR_EQUAL:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("GE", x->data(), y->data());
			node->setReg(x);
			break;
		case AST_EQUAL:
			ALERT(2 != node->length())

			x   = node->child(0);
			y   = node->child(1);
			tmp = node->child(0)->getIndexSize();
			tmp = "" == tmp ? node->child(1)->getIndexSize() : tmp;
			this->emit("EQ", x->data(), y->data(), __IR_DUMMY__, tmp);
			node->setReg(x);
			break;

		case AST_BIT_AND:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("AND", x->data(), y->data());
			node->setReg(x);
			break;
		case AST_BIT_OR:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("OR", x->data(), y->data());
			node->setReg(x);
			break;
		case AST_BIT_XOR:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("XOR", x->data(), y->data());
			node->setReg(x);
			break;

		case AST_ASSIGN:
			ALERT(2 != node->length());

			x = node->child(0);
			y = node->child(1);

			if (AST_BUILDIN_BUFFER != y->type() || x->data() != y->child(0)->data()) {
				if (0 == namescope.count(y->data())) {
					x->vtype(y->vtype());
					this->emit("STORE", x->data(), y->data(), x->getIndex(), x->getIndexSize());
				} else {
					AST *tmp = new AST("");

					tmp->setReg(++this->_regs_);
					x->vtype(namescope[y->data()]);
					this->emit("LOAD",  tmp->data(), y->data(), y->getIndex(), y->getIndexSize());
					this->emit("STORE", x->data(), tmp->data(), x->getIndex(), x->getIndexSize());
					delete tmp;
				}
			}

			if (0 == x->length() || AST_BRACKET_OPEN != x->child(0)->type()) {
				namescope[x->data()] = x->vtype();
				_D(LOG_DEBUG, "%s -> 0x%X", x->data().c_str(), x->vtype());
			}

			break;
		case AST_PRINT:
			/* FIXME - handcode for build-in function: str() */

			tmp = this->randstr(10, "__", "");
			ALERT(0 == node->length());
			x= node->child(0);

			switch(x->vtype()) {
				case VTYPE_BOOLEAN:
					this->emit("ASM", "cmp", x->data(), "0x0");
					this->emit("ASM", "jz", "&" + tmp + "__SHOW_FALSE__");
					this->emit("ASM", "lea", "rsi", "&TRUE");
					this->emit("ASM", "mov", "rdx", "0x06");
					this->emit("ASM", "jmp", "&" + tmp + "__SHOW_TRUE__");
					this->emit("ASM", "asm", tmp + "__SHOW_FALSE__:");
					this->emit("ASM", "lea", "rsi", "&FALSE");
					this->emit("ASM", "mov", "rdx", "0x07");
					this->emit("ASM", "asm", tmp + "__SHOW_TRUE__:");

					this->emit("ASM", "mov", "rax", "0x200018D");
					this->emit("ASM", "mov", "rdi", "0x01");
					this->emit("INTERRUPT");
					break;
				case VTYPE_INTEGER: case VTYPE_OBJECT:
					this->emit("ASM", "mov", "rax", x->data());

					this->emit("ASM", "asm", tmp + "__INT_TO_STR__:");
					this->emit("ASM", "push", "rbp");
					this->emit("ASM", "mov", "rbp", "rsp");
					this->emit("ASM", "sub", "rsp", "0x40");

					this->emit("ASM", "mov", "rsi", "rsp");
					this->emit("ASM", "mov", "rdi", "rsi");
					this->emit("ASM", "mov", "rcx", "0x0A");	/* DIVISOR */

					this->emit("ASM", "cmp", "rax", "0x0");
					this->emit("ASM", "jge", "&" + tmp + "__INT_TO_STR_INNER_LOOP__");
					this->emit("ASM", "neg", "rax");
					this->emit("ASM", "mov", "[rsi]", "0x2D");
					this->emit("ASM", "inc", "rsi");
					this->emit("ASM", "inc", "rdi");

					this->emit("ASM", "asm", tmp + "__INT_TO_STR_INNER_LOOP__:");
					this->emit("ASM", "xor", "rdx", "rdx");
					this->emit("ASM", "div", "rcx");
					this->emit("ASM", "add", "rdx", "0x30");
					this->emit("ASM", "mov", "[rdi]", "rdx");
					this->emit("ASM", "inc", "rdi");
					this->emit("ASM", "cmp", "rax", "0x0");
					this->emit("ASM", "jne", "&" + tmp + "__INT_TO_STR_INNER_LOOP__");

					/* FIXME - hardcode for the build-in function: reserved() */
					this->emit("ASM", "asm", tmp + "__RESERVED__:");
					this->emit("ASM", "mov", "[rdi]", "0x0A");

					this->emit("ASM", "mov", "rdx", "rdi");
					this->emit("ASM", "sub", "rdx", "rsp");
					this->emit("ASM", "inc", "rdx");
					this->emit("ASM", "mov", "rax", "rdi");
					this->emit("ASM", "mov", "rbx", "rsi");
					this->emit("ASM", "dec", "rax");
					this->emit("ASM", "asm", tmp + "__RESERVED_INNER_LOOP__:");
					this->emit("ASM", "mov", "cl", "[rax]");
					this->emit("ASM", "mov", "ch", "[rbx]");
					this->emit("ASM", "mov", "[rax]", "ch");
					this->emit("ASM", "mov", "[rbx]", "cl");
					this->emit("ASM", "dec", "rax");
					this->emit("ASM", "inc", "rbx");
					this->emit("ASM", "cmp", "rax", "rbx");
					this->emit("ASM", "jge", "&" + tmp + "__RESERVED_INNER_LOOP__");

					this->emit("ASM", "mov", "rax", "0x200018D");
					this->emit("ASM", "mov", "rdi", "0x01");
					this->emit("ASM", "mov", "rsi", "rsp");
					this->emit("INTERRUPT");

					this->emit("ASM", "add", "rsp", "0x40");
					this->emit("ASM", "pop", "rbp");
					break;
				case VTYPE_BUFFER:
					snprintf(buf, sizeof(buf), __IR_REG_FMT__, ++this->_regs_);

					this->emit("PARAM", "0x200018D");
					this->emit("PARAM", "0x01");
					this->emit("PARAM", x->data());
					this->emit("LOAD",  buf, x->raw(), "-0x08");
					this->emit("PARAM", buf);
					this->emit("INTERRUPT");
					break;
				default:
					_D(LOG_CRIT, "Not Implemented `%s` [0x%X]", x->data().c_str(), x->vtype());
					break;
			}

			/* FIXME - `print` INTERRUPT */
			break;
		case AST_DELETE:
			ALERT(1 != node->length());

			x = node->child(0);
			snprintf(buf, sizeof(buf), __IR_REG_FMT__, ++this->_regs_);
			switch(x->vtype()) {
				case VTYPE_BUFFER:
					/* FIXME - Should be a built-in function */
					this->emit("PARAM", "0x2000049");
					this->emit("LOAD",  buf, x->data());
					this->emit("SUB",   buf, "0x08");
					this->emit("PARAM", buf);
					this->emit("LOAD",  buf, x->raw(), "-0x08");
					this->emit("ADD",   buf, "0x08");
					this->emit("PARAM", buf);
					this->emit("INTERRUPT");
					break;
				default:
					_D(LOG_CRIT, "Not support delete `%s` 0x%X", x->data().c_str(), x->vtype());
					break;
			}
			break;
		case AST_SYSCALL:
			_D(LOG_CRIT, "Not Implemented %zu", node->length());
			break;

		case AST_NOP:
			this->emit("NOP");
			break;

		/* new CFG */
		case AST_IF:
		case AST_WHILE:
			x = node->child(0);
			node->setReg(x);
			break;
		case AST_ELSE:
			break;
		case AST_BREAK:
			tmp = this->_repeate_label_[this->_repeate_label_.size()-1];
			this->emit("JMP", tmp + "_END");
			break;
		case AST_CONTINUE:
			tmp = this->_repeate_label_[this->_repeate_label_.size()-1];
			this->emit("JMP", tmp);
			break;
		case AST_RETURN:
			switch(node->length()) {
				case 0:
					this->emit("EPILOGUE", ((CFG *)node->root())->varcnt());
					this->emit("RET");
					break;
				case 1:
					x = node->child(0);

					this->emit("EPILOGUE", ((CFG *)node->root())->varcnt());
					this->emit("RET", x->data());
					break;
				case 2:
					ALERT(1 < node->length());
					break;
			}
			break;

		default:
			_D(LOG_CRIT, "Not implemented %s [%X]", node->data().c_str(), node->type());
			exit(-1);
			break;
	}
}
/* wrapper for the IR emitter */
void Zerg::emit(std::string op, std::string dst, std::string src, std::string idx, std::string size) {
	if (this->_only_ir_) {
		if ('#' == op[0]) {
			if ("#!" != op.substr(0, 2)) {
				std::cout << "\n";
			}
			std::cout << op;
		} else if ("ASM" == op) {
			std::cout << std::right << std::setw(4) << "-> ";
			std::cout << std::left  << std::setw(10) << dst;
			std::cout << std::left  << std::setw(6) << src;
			std::cout << std::left  << std::setw(6) << idx;
			std::cout << size;
		} else {
			size_t layout = 12;

			std::cout << std::left << std::setw(layout) <<op;
			if ("" != dst)   std::cout << " " << std::setw(layout) << dst;
			if ("" != src) {
				if (op == "LABEL") src = "\"" + src +  "\"";
			    std::cout << " " << std::setw(layout) << src;
			}
			if ("" != idx) std::cout << " "<< std::setw(layout) << idx;
			if ("" != idx) std::cout << " "<< std::setw(layout) << size;
		}
		std::cout << std::endl;
	} else if ('#' != op[0]) {
		/* call the IR emitter */
		IR::emit(op, dst, src, idx, size);
	}
}

/* register allocation algo. */
std::string Zerg::regalloc(std::string src, std::string size) {
	int cnt = 0;
	std::string tmp;

	if (src == __IR_SYSCALL_REG__) {
		return SYSCALL_REG;
	} else if (1 ==  sscanf(src.c_str(), __IR_REG_FMT__, &cnt)) {
		int pos = 0;
		std::vector<std::string> regs = { REGISTERS };

		if (0 != _alloc_regs_map_.count(src)) {
			/* HACK - Found in cache */
			tmp = _alloc_regs_map_[src];
		} else {
			ALERT(0 == _alloc_regs_.size());

			tmp = _alloc_regs_[0];
			_alloc_regs_.erase(_alloc_regs_.begin());
		}

		/* HACK - resize the register if need */
		if (ZASM_MEM_BYTE         == size) {
			pos = std::find(regs.begin(), regs.end(), tmp) - regs.begin();
			tmp = regs[(pos & 0xE0) + (pos % 8) + 24];
		} else if (ZASM_MEM_WORD  == size) {
			pos = std::find(regs.begin(), regs.end(), tmp) - regs.begin();
			tmp = regs[(pos & 0xE0) + (pos % 8) + 16];
		} else if (ZASM_MEM_DWORD == size) {
			pos = std::find(regs.begin(), regs.end(), tmp) - regs.begin();
			tmp = regs[(pos & 0xE0) + (pos % 8) + 8];
		}

		_D(LOG_INFO, "re-allocate register - %s -> %s", src.c_str(), tmp.c_str());
		_alloc_regs_map_[src] = tmp;
		return tmp;
	}

	return src;
}
void Zerg::regsave(std::string src) {
	int pos = 0;
	std::vector<std::string> regs = { USED_REGISTERS };

	pos = std::find(regs.begin(), regs.end(), src) - regs.begin();
	if (pos != regs.size()) {
		src = regs[(pos & 0xE0) + (pos % 8)];
		_D(LOG_DEBUG, "restore register %s", src.c_str());
		this->_alloc_regs_.push_back(src);
	}
}
std::string Zerg::tmpreg(void) {
	ALERT(1 > _alloc_regs_.size());

	/* NOTE - the template register should NOT same as the next allocate register */
	_D(LOG_DEBUG, "template register `%s`", _alloc_regs_[1].c_str());
	return _alloc_regs_[1];
}
void Zerg::load_namespace(std::map<std::string, VType> &namescope) {
	namescope.clear();

	for (auto it : this->_root_) {
		if (0 == it.second->length()) continue;
		this->_load_namespace_(it.second, namescope);
	}
}
void Zerg::_load_namespace_(CFG *node, std::map<std::string, VType> &namescope) {
	AST *cur = NULL, *tmp = NULL;

	if (NULL != node && 0 != node->length()) {
		tmp = node->child(0);

		switch(tmp->type()) {
			case AST_FUNC:
				cur = tmp->child(0);
				switch(cur->length()) {
					case 2:
						namescope[cur->data()] = VTYPE_OBJECT;
						break;
					case 3:
						if (AST_BUILDIN_BUFFER == cur->child(2)->type()) {
							namescope[cur->data()] = VTYPE_BUFFER;
							break;
						}
					default:
						_D(LOG_BUG, "Should never reach here %zd", cur->length());
						break;
				}
				break;
			default:
				break;
		}

		_load_namespace_(node->nextCFG(true), namescope);
		_load_namespace_(node->nextCFG(false), namescope);
	}
}

