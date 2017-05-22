/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iostream>
#include <iomanip>

#include <unistd.h>
#include "zerg.h"

Zerg::Zerg(std::string dst, ZergArgs *args) : IR(dst, args) {
	this->_labelcnt_	= 0;
	this->_lineno_		= 1;
	this->_regs_		= 0;
	this->_args_		= args;
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
		/* load the built-in library if possible */
		if (0 == access(BUILTIN_LIBRARY, F_OK) && false == args->_no_stdlib_) {
			_D(LOG_INFO, "Load the built-in library `%s`", BUILTIN_LIBRARY);
			this->lexer(BUILTIN_LIBRARY);
		}
		this->lexer(src);

		this->emit("#! /usr/bin/env zgr");
		this->emit("#! ZERG IR - v" __IR_VERSION__);
		this->emit("ASM", "call", __IR_REFERENCE__ ZASM_ENTRY_POINT);
		this->emit("ASM", "mov", "rax", "0x2000001");
		this->emit("INTERRUPT");

		if (0 == this->_root_.count(CFG_MAIN)) {
			this->emit("LABEL", ZASM_ENTRY_POINT);
			this->emit("RET");
		}

		for (auto it : this->_root_) {
			/* compile subroutine */
			std::map<std::string, VType> namescope;
			_D(LOG_INFO, "compile subroutine `%s`", it.first.c_str());

			/* reset the allocated register */
			this->_alloc_regs_map_.clear();
			this->_alloc_regs_ = { USED_REGISTERS };

			this->emit("# Sub-Routine - " + it.first);
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

	this->emit("LABEL", CFG_MAIN == node->label() ? ZASM_ENTRY_POINT : node->label());

	if (0 == node->length()) {
		/* need NOT process this node */
		/* PROLOGUE */
		this->emit("PROLOGUE", node->varcnt());

		this->_compileCFG_(node->nextCFG(true), namescope);
		this->_compileCFG_(node->nextCFG(false), namescope);

		/* EPILOGUE */
		this->emit("EPILOGUE", node->varcnt());
	} else if (AST_CLASS == node->child(0)->type()) {
		/* generate the class code-block */
		char buff[BUFSIZ] = {0}, name[BUFSIZ] = {0};
		size_t bufsiz = 0;

		/* PROLOGUE */
		this->emit("PROLOGUE", node->varcnt());

		namescope[node->label()] = VTYPE_CLASS;
		bufsiz  = (node->child(0)->length()-1) * 2;
		bufsiz += 2;
		bufsiz *= PARAM_SIZE;
		snprintf(buff, sizeof(buff), "0x%lX", bufsiz);
		node->child(0)->setReg(++this->_regs_);

		this->emit("PARAM", buff);
		this->emit("CALL", "buffer", "1");
		this->emit("STORE", node->child(0)->data(), __IR_SYSCALL_REG__);
		/* object type */
		snprintf(buff, sizeof(buff), "0x%x", 0x0);
		/* FIXME - type */
		this->emit("STORE", node->child(0)->data(), "0x80000000", buff, ZASM_MEM_DWORD);
		/* reference count */
		snprintf(buff, sizeof(buff), "0x%x", PARAM_SIZE/2);
		this->emit("STORE", node->child(0)->data(), "0x1", buff, ZASM_MEM_DWORD);
		/* class name */
		snprintf(buff, sizeof(buff), "0x%x", PARAM_SIZE);
		snprintf(name, sizeof(name), __IR_REFERENCE__ __IR_CLS_NAME__,
												node->child(0)->child(0)->raw().c_str());
		this->emit("STORE", node->child(0)->data(), name, buff, ZASM_MEM_QWORD);
		this->_symb_.push_back(std::make_pair(name+1, node->child(0)->child(0)->raw()));

		/* set the properties */
		this->_compileCFG_(node, namescope);

		/* EPILOGUE */
		this->emit("EPILOGUE", node->varcnt());
		this->emit("RET", node->child(0)->data());
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
	#if defined(DEBUG_CFG) || defined(DEBUG)
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

	/* main - emit IR*/
	this->emitIR(node, namescope);

	if (node->isCondit()) {		/* CONDITION NODE */
		std::string label;

		if (NULL == node->nextCFG(false)) {
			label = node->label() + ".end";
		} else {
			label = node->nextCFG(false)->label();
		}
		this->emit("JMPIF", label, node->data());
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
			_D(LOG_DEBUG, "set label %s.end", node->prev()->label().c_str());
			this->emit("LABEL", node->prev()->label() + ".end");
		}

		if (0 != node->length() && AST_WHILE == node->child(0)->type()) {
			this->_repeate_label_.pop_back();
		}
	}
}
void Zerg::emitIR(AST *node, std::map<std::string, VType> &namescope) {
	static std::vector<std::string> stack;

	size_t pos = 0;
	char buf[BUFSIZ] = {0};
	std::string tmp;
	AST *x = NULL, *y = NULL, *cur = NULL;

	node->setEmitted();
	/* process first if need */
	switch(node->type()) {
		case AST_ASSIGN:
			ALERT(2 != node->length());

			x = node->child(0);
			y = node->child(1);

			this->emitIR(y, namescope);
			this->emitIR(x, namescope);
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

				if (VTYPE_OBJECT == y->vtype()) {
					this->_obj_instance_[x->raw()] = y->raw();
				}
			}

			return ;
		case AST_INC: case AST_DEC:
			if (1 != node->length() || AST_IDENTIFIER != node->child(0)->type()) {
				x = node->child(0);

				_D(LOG_CRIT, "Not allow the syntax for `%s` on %s",
												AST_INC == node->type() ? "++" : "--",
												x->data().c_str());
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
			this->emit("JMPIF", tmp, x->data());
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
						if (0 == namescope.count(node->data())) {
							_D(LOG_CRIT, "`%s` not define", node->data().c_str());
							return ;
						}
						node->vtype(namescope[node->data()]);

						if (VTYPE_CLASS == node->vtype()) {
							/* create an instance */
							node->vtype(VTYPE_OBJECT);
						}

						x = node->child(0);
						for (ssize_t i = 0; i < x->length(); ++i) {
							this->emitIR(x->child(i), namescope);
							this->emit("PARAM", x->child(i)->data());
						}

						snprintf(buf, sizeof(buf), "%lu", x->length());
						this->emit("CALL", node->data(), buf);
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

			for (ssize_t i = 0; i < node->length(); ++i) {
				this->emitIR(node->child(i), namescope);
			}
			break;
		case AST_SYSCALL:
			if (2 == node->length() && AST_PARENTHESES_OPEN == node->child(0)->type()) {
				for (ssize_t i = 0; i < node->child(0)->length(); ++i) {
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

			for (ssize_t i = 0; i < node->length(); ++i) {
				this->emitIR(node->child(i), namescope);
			}
			break;
		case AST_FUNC:
			cur = node->child(0);
			x   = cur->child(0);

			for (ssize_t i = 0; i < x->length(); ++i) {
				char idx[BUFSIZ] = {0};

				y = x->child(i);
				y->setReg(++this->_regs_);
				snprintf(idx, sizeof(idx), "0x%lX", (x->length()+1-i) * 0x08);
				this->emit("LOAD", y->data(), __IR_FUNC_STACK__, idx, ZASM_MEM_QWORD);
				this->emit("STORE", y->raw(), y->data());
				namescope[y->raw()] = VTYPE_PARAM;
			}

			return ;
		case AST_CLASS:
			ALERT(0 == node->length() || 2 != node->child(0)->length());

			cur = node->child(0);
			if (0 != cur->child(0)->length()) {
				/* inherit other class */
				_D(LOG_CRIT, "Not support inherit on class `%s`", cur->data().c_str());
			} else if (1 >= node->length()) {
				/* no any property */
				_D(LOG_CRIT, "class `%s` syntax error", cur->data().c_str());
			}

			for (int i = 0; i < node->length()-1; ++i) {
				char buff[BUFSIZ] = {0}, name[BUFSIZ] = {0};

				cur = node->child(i+1);
				switch(cur->type()) {
					case AST_NOP:
						/* NOP */
						break;
					case AST_ASSIGN:
						ALERT(2 != cur->length());

						x = cur->child(0);
						y = cur->child(1);
						this->emitIR(y, namescope);
						snprintf(buff, sizeof(buff), "0x%X", (i+1)*2*PARAM_SIZE);
						this->emit("STORE", node->data(), y->data(), buff, ZASM_MEM_QWORD);

						snprintf(buff, sizeof(buff), "0x%X", ((i+1)*2+1)*PARAM_SIZE);
						snprintf(name, sizeof(name),  __IR_REFERENCE__ __IR_CLS_REG__,
															node->child(0)->raw().c_str(),
															x->raw().c_str());

						this->emit("STORE", node->data(), name, buff, ZASM_MEM_QWORD);
						/* HACK */
						this->_symb_.push_back(std::make_pair(name+1, x->raw()));
						/* set the property map */
						this->_obj_property_[node->child(0)->data()].push_back(x->data());
						break;
					case AST_FUNC:
					default:
						_D(LOG_CRIT, "Not Implemented set property `%s`",
																	cur->data().c_str());
						break;
				}
			}

			#if defined(DEBUG) || defined(DEBUG_CLASS)
			_D(LOG_BUG, "class %s with properties", node->child(0)->data().c_str());
			for (unsigned int i = 0; i < this->_obj_property_.size(); ++i) {
				std::string name = this->_obj_property_[node->child(0)->data()][i];

				_D(LOG_BUG, "#%2u %s", i, name.c_str());
			}
			#endif /* DEBUG_CLASS */

			return ;
		case AST_OBJECT:
			this->emit("PARAM", "0x12");
			this->emit("CALL", "buffer");
			node->setReg(SYSCALL_REG);
			node->vtype(VTYPE_OBJECT);
			/* set reference count */
			this->emit("STORE", node->data(), "0x01", "0x02", "word");
			return ;
		case AST_DOT:
			ALERT(2 != node->length());

			x = node->child(0);
			y = node->child(1);

			switch(x->type()) {
				case AST_IDENTIFIER:
					if (0 == this->_obj_instance_.count(x->raw())) {
						_D(LOG_CRIT, "class `%s` not declare", x->data().c_str());
						break;
					}

					tmp = this->_obj_instance_[x->raw()];
					pos = std::find(_obj_property_[tmp].begin(),
									_obj_property_[tmp].end(),
									y->data()) - _obj_property_[tmp].begin();

					if (pos == _obj_property_[tmp].size()) {
						_D(LOG_CRIT, "property `%s` not defined on %s (#%lu)",
													y->data().c_str(), tmp.c_str(), pos);
						break;
					}

					snprintf(buf, sizeof(buf), "0x%zX", (pos + 2) * PARAM_SIZE);
					this->emitIR(x, namescope);

					/* FIXME */
					node->setReg(++this->_regs_);
					node->vtype(VTYPE_INTEGER);
					this->emit("LOAD", node->data(), x->data(), buf, ZASM_MEM_QWORD);
					return ;
				default:
					_D(LOG_CRIT, "Not Implemented search property");
					break;
			}

			break;
		default:
			/* Run DFS */
			for (ssize_t i = 0; i < node->length(); ++i) {
				this->emitIR(node->child(i), namescope);
			}
			break;
	}

	/* translate AST to IR */
	_D(LOG_INFO, "emit IR on %s (vars: #%zu)", node->data().c_str(), namescope.size());
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

		case AST_PRINT:
			/* FIXME - hand-code for build-in function: str() */

			tmp = this->randstr(10, "__");
			ALERT(0 == node->length());
			x= node->child(0);

			switch(x->vtype()) {
				case VTYPE_BOOLEAN:
					this->emit("ASM", "cmp", x->data(), "0x0");
					this->emit("ASM", "jz", __IR_REFERENCE__ + tmp + "__SHOW_FALSE__");
					this->emit("ASM", "lea", "rsi", "&TRUE");
					this->emit("ASM", "mov", "rdx", "0x06");
					this->emit("ASM", "jmp", __IR_REFERENCE__ + tmp + "__SHOW_TRUE__");
					this->emit("ASM", "asm", tmp + "__SHOW_FALSE__:");
					this->emit("ASM", "lea", "rsi", "&FALSE");
					this->emit("ASM", "mov", "rdx", "0x07");
					this->emit("ASM", "asm", tmp + "__SHOW_TRUE__:");

					this->emit("ASM", "mov", "rax", "0x200018D");
					this->emit("ASM", "mov", "rdi", "0x01");
					this->emit("INTERRUPT");
					break;
				case VTYPE_INTEGER:
					this->emit("PARAM", x->data());
					this->emit("CALL", "str", "1");
					this->emit("ASM", "mov", "rsi", SYSCALL_REG);
					this->emit("PARAM", SYSCALL_REG);
					this->emit("CALL", "strlen", "1");
					this->emit("ASM", "mov", "rdx", SYSCALL_REG);

					/* HACK - add extra '\n' */
					tmp = tmpreg();
					this->emit("ASM", "mov", tmp, "rsi");
					this->emit("ASM", "add", tmp, "rdx");
					this->emit("ASM", "mov", "[" + tmp + "]", "0x0A");
					this->emit("INC", "rdx");

					this->emit("ASM", "mov", "rax", "0x200018D");
					this->emit("ASM", "mov", "rdi", "0x01");
					this->emit("INTERRUPT");

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
				case VTYPE_CLASS:
					_D(LOG_CRIT, "Not Implemented print class `%s`", x->raw().c_str());
					break;
				case VTYPE_OBJECT:
					/* FIXME- Correct way is call the __str__ property */
					this->emit("PARAM", "0x200018D");
					this->emit("PARAM", "0x01");
					snprintf(buf, sizeof(buf), "0x%x", PARAM_SIZE);
					this->emit("LOAD", x->data(), x->data(), buf, ZASM_MEM_QWORD);
					this->emit("PARAM", x->data());
					this->emit("PARAM", x->data());
					this->emit("CALL",  "strlen", "1");
					this->emit("PARAM", __IR_SYSCALL_REG__);
					this->emit("INTERRUPT");
					break;
				default:
					_D(LOG_CRIT, "Not Implemented print `%s` [0x%X]",
															x->raw().c_str(), x->vtype());
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
			this->emit("JMP", tmp + ".end");
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
void Zerg::emit(STRING op, STRING dst, STRING src, STRING idx, STRING size) {
	if (this->_args_->_only_ir_) {
		if ('#' == op[0]) {
			if ("#!" != op.substr(0, 2)) {
				std::cout << "\n";
			}
			std::cout << op;
		} else if ("ASM" == op) {
			std::cout << std::right << std::setw(4) << "-> ";
			std::cout << std::left  << std::setw(10) << dst;
			std::cout << std::left  << std::setw(10) << src;
			std::cout << std::left  << std::setw(10) << idx;
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

		_D(LOG_REGISTER_ALLOC, "%8s -> %4s", src.c_str(), tmp.c_str());
		_alloc_regs_map_[src] = tmp;
		return tmp;
	}

	return src;
}
void Zerg::regsave(std::string src) {
	unsigned int pos = 0;
	std::vector<std::string> regs = { REGISTERS }, used = { USED_REGISTERS };

	pos = std::find(regs.begin(), regs.end(), src) - regs.begin();
	if (pos != regs.size()) {
		src = regs[(pos & 0xE0) + (pos % 8)];

		if (used.end() != std::find(used.begin(), used.end(), src)) {
			_D(LOG_REGISTER_ALLOC, "%8s <- %4s", "", src.c_str());
			this->_alloc_regs_.push_back(src);
		}
	}
}
std::string Zerg::tmpreg(void) {
	ALERT(1 >= _alloc_regs_.size());

	/* NOTE - the template register should NOT same as the next allocate register */
	_D(LOG_REGISTER_ALLOC, "%8s -> %4s #%zu",
									"", _alloc_regs_[1].c_str(), _alloc_regs_.size());
	return _alloc_regs_[1];
}
void Zerg::resetreg(void) {
	/* reset the register usage */
	_D(LOG_REGISTER_ALLOC, "%8s <- reset all", "");
	_alloc_regs_ = { USED_REGISTERS };
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
						/* ── cur
						 *    ├── (
						 *    └── )
						 */
						_D(LOG_DEBUG, "set %s as unknown", cur->data().c_str());
						namescope[cur->data()] = VTYPE_OBJECT;
						break;
					case 3:
						if (AST_BUILDIN_BUFFER == cur->child(2)->type()) {
							/* ── cur
							 *    ├── (
							 *    ├── )
							 *    └── __buffer__
							 */
							_D(LOG_DEBUG, "set %s as buffer", cur->data().c_str());
							namescope[cur->data()] = VTYPE_BUFFER;
							break;
						}
					default:
						_D(LOG_ERROR, "Should never reach here %zd", cur->length());
						break;
				}
				break;
			case AST_CLASS:
				cur = tmp->child(0);
				_D(LOG_DEBUG, "set %s as class", cur->data().c_str());
				namescope[cur->data()] = VTYPE_CLASS;
				break;
			default:
				break;
		}

		_load_namespace_(node->nextCFG(true), namescope);
		_load_namespace_(node->nextCFG(false), namescope);
	}
}

