/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include "zerg.h"

Zerg::Zerg(std::string dst, off_t entry) : ParsingTable(), IR(dst, entry) {
	this->_labelcnt_ = 0;
}
Zerg::~Zerg() {
	/* delete the AST node */
	for (auto it : this->_root_) {
		delete it.second;
	}
}

void Zerg::compile(std::string src, bool only_ir) {
	this->_only_ir_ = only_ir;

	this->lexer(src);

	this->emit("PROLOGUE");
	this->compileCFG(this->_root_[CFG_MAIN]);
	for (auto it : this->_root_) {
		if (it.first != CFG_MAIN) {
			/* compile subroutine */
			_D(LOG_WARNING, "compile subroutine `%s`", it.first.c_str());
			this->emit("# Sub-Routine - " + it.first);
			this->emit("LABEL", it.first);
			this->compileCFG(it.second);
			this->emit("RET");
		}
	}
	this->emit("EPILOGUE");

	/* Dump all symbol at end of CFG */
	_D(LOG_INFO, "dump all symbols");
	this->emit("# Dump all symbols");
	for (auto it : this->_symb_) {
		this->emit("LABEL", it.first, it.second);
	}
}
void Zerg::compileCFG(CFG *node) {
	CFG *tmp = NULL;

	if (NULL != node && ! node->isEmmited()) {
		#ifdef DEBUG
		std::cout << *node << std::endl;
		#endif

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
		if (node->isCondit() || node->isRefed()){
			/* set label if need */
			this->emit("LABEL", node->label());
		}

		/* main logical to generate IR */
		this->emitIR(node);

		if (node->isCondit()) {				/* CONDITION NODE */
			std::string op = node->child(0)->data();

			tmp = node->nextCFG(false);
			if ("<" == op) {
				/* if x < y not not established*/
				this->emit("JGTE", "&" + tmp->label());
			} else {
				_D(LOG_CRIT, "Not Implemented `%s`", op.c_str());
				exit(-1);
			}
		} else if (node->isBranch() && NULL != (tmp = node->nextCFG(true))) {
			/* end of branch node, jump to next stage */
			this->emit("JMP","&" + tmp->label());
		}

		/* process other stage in CFG */
		this->compileCFG(node->nextCFG(true));
		this->compileCFG(node->nextCFG(false));
	}
}
void Zerg::emitIR(AST *node) {
	static int regs = 0;
	static std::vector<std::string> stack;

	std::string tmp;
	AST *x = NULL, *y = NULL, *cur = NULL;

	/* process first if need */
	switch(node->type()) {
		case AST_IDENTIFIER:
		case AST_SYSCALL:
			if (2 == node->length() && AST_PARENTHESES_OPEN == node->child(0)->type()) {
				for (size_t i = 0; i < node->child(0)->length(); ++i) {
					cur = node->child(0)->child(i);
					this->emitIR(cur);
					switch(cur->type()) {
						case AST_STRING:
							this->emit("PARAM", "&" + cur->data());
							break;
						default:
							this->emit("PARAM", cur->data());
							break;
					}
				}
				this->emit("INTERRUPT");
				return;
			}
		default:
			/* Run DFS */
			for (size_t i = 0; i < node->length(); ++i) {
				this->emitIR(node->child(i));
			}
			break;
	}

	/* translate AST to IR */
	_D(LOG_DEBUG, "emit IR on %s", node->data().c_str());
	switch(node->type()) {
		case AST_ROOT:
			/* NOP */
			break;
		case AST_NUMBER:
			tmp = node->data();
			node->setReg(++regs);
			this->emit("STORE", node->data(), tmp);
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
					tmp = node->data();
					node->setReg(++regs);

					this->emit("LOAD", node->data(), tmp, __IR_LOCAL_VAR__);
					break;
			}
			break;
		case AST_ADD:
			ALERT(2 > node->length())
			x = node->child(0);
			y = node->child(1);
			this->emit("ADD", x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case AST_SUB:
			ALERT(2 > node->length())
			x = node->child(0);
			y = node->child(1);
			this->emit("SUB", x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case AST_MUL:
			ALERT(2 > node->length())
			x = node->child(0);
			y = node->child(1);
			this->emit("MUL", x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case AST_DIV:
			ALERT(2 > node->length())
			x = node->child(0);
			y = node->child(1);
			this->emit("DIV", x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case AST_MOD:
			ALERT(2 > node->length())
			x = node->child(0);
			y = node->child(1);
			this->emit("REM", x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case AST_ASSIGN:
			ALERT(2 != node->length());

			x = node->child(0);
			y = node->child(1);

			this->emit("STORE", x->data(), y->data(), __IR_LOCAL_VAR__);
			break;
		case AST_FUNCCALL:
			switch (node->length()) {
				case 0:
					_D(LOG_CRIT, "Syntax Error !");
					exit(-1);
					break;
				case 1:
					this->emit("CALL", "&" + node->child(0)->data());
					break;
				default:
					_D(LOG_CRIT, "Not Implemented");
					exit(-1);
					break;
			}
			break;
		case AST_SYSCALL:
			_D(LOG_CRIT, "Not Implemented %zu", node->length());
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
	if ('#' == op[0]) {
		/* dump the comment */
		if (this->_only_ir_) std::cout << "\n" << op << std::endl;
	} else if (this->_only_ir_) {
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
