/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include "zerg.h"

Zerg::Zerg(std::string dst, off_t entry) : IR(dst, entry) {
	this->_labelcnt_ = 0;
	this->_root_     = NULL;
}
Zerg::~Zerg() {
	/* delete the AST node */
	delete this->_root_;
}

void Zerg::compile(std::string src, bool only_ir) {
	this->_only_ir_ = only_ir;

	CFG *root = NULL, *condi = NULL, *payload = NULL, *exit = NULL;

	root = new CFG("root");
	root->insert("=");
	root->child(0)->insert("_VAR");
	root->child(0)->insert("0x0");

	condi = new CFG("branch");
	condi->insert("<");
	condi->child(0)->insert("_VAR");
	condi->child(0)->insert("0x03");

	/* syscall (0x2000004, 0x02, "Zerg IR Countdown\n", 0x0C) */
	payload = new CFG("payload");
	payload->insert("syscall");
	payload->child(0)->insert("0x2000004");
	payload->child(0)->insert("0x01");
	payload->child(0)->insert("'Zerg IR Countdown\\n'");
	payload->child(0)->insert("0x12");

	payload->insert("=");
	payload->child(1)->insert("_VAR");
	payload->child(1)->insert("+");
	payload->child(1)->child(1)->insert("_VAR");
	payload->child(1)->child(1)->insert("0x1");


	/* syscall (0x2000001, 0x04) */
	exit = new CFG("exit");
	exit->insert("syscall");
	exit->child(0)->insert("0x2000001");
	exit->child(0)->insert("0x04");

	do { /* relation of CFG */
		root->passto(condi);
		condi->branch(payload, exit);
		payload->branch(condi, NULL);
	} while (0);

	this->_root_ = root;
	this->DFS(root);
}

/* Private Properties */
void Zerg::lexer(void) {
	_D(LOG_CRIT, "Not Implemented");
	exit(-1);
}
void Zerg::parser(void) {
	_D(LOG_CRIT, "Not Implemented");
	exit(-1);
}
void Zerg::DFS(CFG *node) {
	CFG *tmp = NULL;

	if (NULL != node && ! node->isEmmited()) {
		#ifdef DEBUG
		std::cout << *node << std::endl;
		#endif

		if ("" != node->label()) {
			this->emit("LABEL", node->label());
		}
		this->DFS((AST *)node);

		/* else-part */
		if (NULL != node->nextCFG(false)) {
			std::string op = node->child(0)->data();

			/* NOTE - the jump logical is reversed */
			tmp = node->nextCFG(false);
			if ("<" == op) {
				this->emit("JGTE", "&" + tmp->label());
			} else {
				_D(LOG_CRIT, "Not Implemented %s", op.c_str());
			}
		}

		if (node->isBranch() && node->nextCFG(true)) {
			tmp = node->nextCFG(true);
			/* end of branch node, jump to next stage */
			this->emit("JMP","&" + tmp->label());
		}
		this->DFS(node->nextCFG(true));
		this->DFS(node->nextCFG(false));
	}

	if (node == this->_root_) {
		_D(LOG_INFO, "dump all symbols");
		for (auto it : this->_symb_) {
			this->emit("LABEL", it.first, it.second);
		}
	}
}
void Zerg::DFS(AST *node) {
	/* main DFS logical */
	for (ssize_t i = 0; i < node->length(); ++i) {
		this->DFS(node->child(i));
	}
	this->compileIR(node);
}
void Zerg::compileIR(AST *node) {
	static int regs = 0;
	static std::vector<std::string> stack;
	std::string tmp;

	_D(LOG_INFO, "emit IR on %s", node->data().c_str());

	/* translate AST to IR */
	switch(node->type()) {
		case AST_ROOT:
		case AST_NUMBER:
			/* NOP */
			break;
		case AST_IDENTIFIER:
			/* Load into register */
			if (AST_ASSIGN != node->parent()->type()) {
				size_t pos;
				tmp = node->data();
				node->setReg(++regs);

				pos = std::find(stack.begin(), stack.end(), tmp) - stack.begin();
				if (stack.size() != pos) {
					char nr[BUFSIZ] = {0};

					snprintf(nr, sizeof(nr), "0x%zX", pos);
					this->emit("LOAD", node->data(), "STACK", nr);
				} else {
					this->emit("LOAD", node->data(), tmp);
				}
			}
			break;
		case AST_STRING:
			tmp = node->data();
			node->setLabel(++this->_labelcnt_);
			this->_symb_.push_back(std::make_pair(node->data(), tmp));
			break;
		case AST_INTERRUPT:
			for (ssize_t i = 0; i < node->length(); ++i) {
				AST *child = node->child(i);

				switch(child->type()) {
					case AST_STRING:
						this->emit("PARAM", "&" + node->child(i)->data());
						break;
					default:
						this->emit("PARAM", node->child(i)->data());
						break;
				}
			}
			this->emit("INTERRUPT");
			break;
		case AST_ASSIGN:
			if (2 == node->length()) {
				AST *src = node->child(1), *dst = node->child(0);

				if ('_' == dst->data()[0]) {
					size_t pos;
					std::string tmp = dst->data();
					/* local variable */

					pos = std::find(stack.begin(), stack.end(), tmp) - stack.begin();

					if (0 != stack.size() && pos != stack.size()) {
						char nr[BUFSIZ] = {0};

						snprintf(nr, sizeof(nr), "0x%zX", pos);
						this->emit("STORE", "STACK", src->data(), nr);
					} else {
						stack.push_back(dst->data());
						this->emit("STORE", "STACK", src->data());
					}

					_D(LOG_DEBUG, "save %s on stack %zu", dst->data().c_str(), pos);
				} else {
					this->emit("STORE", dst->data(), src->data());
				}
			} else {
				_D(LOG_CRIT, "Not Implemented %zu", node->length());
				exit(-1);
			}
			break;
		case AST_OPERATORS:
			if ("<" == node->data()) {
				AST *x = NULL, *y = NULL;

				x = node->child(0);
				y = node->child(1);
				this->emit("CMP", x->data(), y->data());
			} else if ("+" == node->data()) {
				AST *x = NULL, *y = NULL;

				x = node->child(0);
				y = node->child(1);
				this->emit("ADD", x->data(), y->data());
				node->setReg(x->getReg());
			} else {
				_D(LOG_CRIT, "Not Implemented %s", node->data().c_str());
				exit(-1);
			}
			break;
		default:
			_D(LOG_CRIT, "Not implemented %s [%X]", node->data().c_str(), node->type());
			exit(-1);
			break;
	}

	node->setEmitted();
}
/* wrapper for the IR emitter */
void Zerg::emit(std::string op, std::string dst, std::string src, std::string extra) {
	if (this->_only_ir_) {
		std::cout << "(" << std::left << std::setw(10) <<op;
		if ("" != dst)   std::cout << ", " << dst;
		if ("" != src)   std::cout << ", " << src;
		if ("" != extra) std::cout << ", " << extra;
		std::cout << ")" << std::endl;
	} else {
		/* call the IR emitter */
		IR::emit(op, dst, src, extra);
	}
}

/* register allocation algo. */
std::string Zerg::regalloc(std::string src) {
	int cnt = 0;
	std::vector<std::string> regs = {"rax", "rcx", "rdx", "rbx", "rdi", "rsi"};

	/* FIXME - the algo. is not correct! */
	/* FIXME - need more robust */
	sscanf(src.c_str(), __IR_REG_FMT__, &cnt);
	if (0 != cnt) {
		return regs[cnt-1];
	}

	return src;
}
