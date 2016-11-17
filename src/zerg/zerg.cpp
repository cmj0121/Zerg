/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include "zerg.h"

#define PARAM_SIZE	0x08

Zerg::Zerg(std::string dst, off_t entry) : IR(dst, entry) {
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

	this->emit("# ZERG IR - v" __IR_VERSION__);
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

	_D(LOG_INFO, "dump all symbols");
	this->_symb_.push_back(std::make_pair("TRUE",  "True\\n"));
	this->_symb_.push_back(std::make_pair("FALSE", "False\\n"));
	if (0 != this->_symb_.size()) {
		this->emit("# Dump all symbols");
		for (auto it : this->_symb_) {
			this->emit("LABEL", it.first, it.second);
		}
	}
}
void Zerg::compileCFG(CFG *node) {
	int cnt = 0;
	char cntvar[64] = {0};
	CFG *tmp = NULL;

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

	if (NULL == node || node->isEmmited()) {
		_D(LOG_DEBUG, "already processed");
		return ;
	}

	#ifdef DEBUG
	std::cout << *node << std::endl;
	#endif

	/* PROLOGUE */
	for (size_t i = 0; i < ((AST *)node)->length(); ++i) {
		AST *child = (AST *)node->child(i);

		if (AST_ASSIGN == child->type()) cnt ++;
	}

	if (0 != cnt) snprintf(cntvar, sizeof(cntvar), "0x%X", cnt*PARAM_SIZE);
	this->emit("PROLOGUE", cntvar);

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

	/* EPILOGUE */
	this->emit("EPILOGUE", cntvar);
}
void Zerg::emitIR(AST *node) {
	static int regs = 0;
	static std::vector<std::string> stack;

	std::string tmp;
	AST *x = NULL, *y = NULL, *cur = NULL;

	/* process first if need */
	switch(node->type()) {
		case AST_LOG_AND:
			ALERT(2 != node->length());

			tmp = IR::randstr();
			x = node->child(0);
			y = node->child(1);
			this->emitIR(x);
			this->emit("JMP_FALSE", tmp, x->data());
			this->emitIR(y);
			this->emit("STORE", x->data(), y->data());
			this->emit("LABEL", tmp);
			node->setReg(x->getReg());
			return;
		case AST_LOG_OR:
			ALERT(2 != node->length());

			tmp = IR::randstr();
			x = node->child(0);
			y = node->child(1);
			this->emitIR(x);
			this->emit("JMP_TRUE", tmp, x->data());
			this->emitIR(y);
			this->emit("STORE", x->data(), y->data());
			this->emit("LABEL", tmp);
			node->setReg(x->getReg());
			return;
		case AST_LOG_XOR:
			ALERT(2 != node->length());

			x = node->child(0);
			y = node->child(1);
			this->emitIR(x);
			this->emitIR(y);
			this->emit("XOR", x->data(), y->data());
			node->setReg(x->getReg());
			return ;
		case AST_LOG_NOT:
			ALERT(1 != node->length());

			x = node->child(0);
			this->emitIR(x);
			this->emit("XOR", x->data(), "0x1");
			node->setReg(x->getReg());
			return;
		case AST_IDENTIFIER:
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
							this->emitIR(cur);
							this->emit("PARAM", "&" + cur->data());
							break;
						default:
							this->emitIR(cur);
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
			ALERT(2 < node->length() || 0 == node->length());

			switch(node->length()) {
				case 1:
					/* NOP */
					x = node->child(0);
					node->setReg(x->getReg());
					break;
					break;
				case 2:
					x = node->child(0);
					y = node->child(1);
					this->emit("ADD", x->data(), y->data());
					node->setReg(x->getReg());
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
					this->emit("NEG", x->data());
					node->setReg(x->getReg());
					break;
				case 2:
					x = node->child(0);
					y = node->child(1);
					this->emit("SUB", x->data(), y->data());
					node->setReg(x->getReg());
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
					this->emit("NOT", x->data());
					node->setReg(x->getReg());
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
			node->setReg(x->getReg());
			break;
		case AST_DIV:
			ALERT(2 != node->length())
			x = node->child(0);
			y = node->child(1);
			this->emit("DIV", x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case AST_MOD:
			ALERT(2 != node->length())
			x = node->child(0);
			y = node->child(1);
			this->emit("REM", x->data(), y->data());
			node->setReg(x->getReg());
			break;

		case AST_LSHT:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("SHL", x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case AST_RSHT:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("SHR", x->data(), y->data());
			node->setReg(x->getReg());
			break;

		case AST_LESS:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("LS", x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case AST_LESS_OR_EQUAL:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("LE", x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case AST_GRATE:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("GT", x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case AST_GRATE_OR_EQUAL:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("GE", x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case AST_EQUAL:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("EQ", x->data(), y->data());
			node->setReg(x->getReg());
			break;

		case AST_BIT_AND:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("AND", x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case AST_BIT_OR:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("OR", x->data(), y->data());
			node->setReg(x->getReg());
			break;
		case AST_BIT_XOR:
			ALERT(2 != node->length())

			x = node->child(0);
			y = node->child(1);
			this->emit("XOR", x->data(), y->data());
			node->setReg(x->getReg());
			break;

		case AST_ASSIGN:
			ALERT(2 != node->length());

			x = node->child(0);
			y = node->child(1);

			this->emit("STORE", x->data(), y->data(), __IR_LOCAL_VAR__);
			break;
		case AST_PRINT:
			/* FIXME - hardcode for build-in funciton: str() */

			ALERT(0 == node->length());
			x= node->child(0);

			switch(x->type()) {
				case AST_LESS:   case AST_LESS_OR_EQUAL:
				case AST_GRATE:  case AST_GRATE_OR_EQUAL:
				case AST_EQUAL:
				case AST_LOG_AND: case AST_LOG_OR: case AST_LOG_XOR: case AST_LOG_NOT:
					this->emit("ASM", "cmp", x->data(), "0x0");
					this->emit("ASM", "jz", "&__SHOW_FALSE__");
					this->emit("ASM", "lea", "rsi", "&TRUE");
					this->emit("ASM", "mov", "rdx", "0x06");
					this->emit("ASM", "jmp", "&__SHOW_TRUE__");
					this->emit("ASM", "asm", "__SHOW_FALSE__:");
					this->emit("ASM", "lea", "rsi", "&FALSE");
					this->emit("ASM", "mov", "rdx", "0x07");
					this->emit("ASM", "asm", "__SHOW_TRUE__:");

					this->emit("ASM", "mov", "rax", "0x2000004");
					this->emit("ASM", "mov", "rdi", "0x01");
					this->emit("INTERRUPT");
					break;
				default:
					this->emit("ASM", "mov", "rax", x->data());

					this->emit("ASM", "asm", "__INT_TO_STR__:");
					this->emit("ASM", "mov", "rsi", "rsp");
					this->emit("ASM", "mov", "rdi", "rsi");
					this->emit("ASM", "mov", "rcx", "0x0A");	/* DIVISOR */

					this->emit("ASM", "cmp", "rax", "0x0");
					this->emit("ASM", "jge", "&__INT_TO_STR_INNER_LOOP__");
					this->emit("ASM", "neg", "rax");
					this->emit("ASM", "mov", "[rsi]", "0x2D");
					this->emit("ASM", "inc", "rsi");
					this->emit("ASM", "inc", "rdi");

					this->emit("ASM", "asm", "__INT_TO_STR_INNER_LOOP__:");
					this->emit("ASM", "xor", "rdx", "rdx");
					this->emit("ASM", "div", "rcx");
					this->emit("ASM", "add", "rdx", "0x30");
					this->emit("ASM", "mov", "[rdi]", "rdx");
					this->emit("ASM", "inc", "rdi");
					this->emit("ASM", "cmp", "rax", "0x0");
					this->emit("ASM", "jne", "&__INT_TO_STR_INNER_LOOP__");

					/* FIXME - hardcode for the build-in function: reserved() */
					this->emit("ASM", "asm", "__RESERVED__:");
					this->emit("ASM", "mov", "[rdi]", "0x0A");

					this->emit("ASM", "mov", "rdx", "rdi");
					this->emit("ASM", "sub", "rdx", "rsp");
					this->emit("ASM", "inc", "rdx");
					this->emit("ASM", "mov", "rax", "rdi");
					this->emit("ASM", "mov", "rbx", "rsi");
					this->emit("ASM", "dec", "rax");
					this->emit("ASM", "asm", "__RESERVED_INNER_LOOP__:");
					this->emit("ASM", "mov", "cl", "[rax]");
					this->emit("ASM", "mov", "ch", "[rbx]");
					this->emit("ASM", "mov", "[rax]", "ch");
					this->emit("ASM", "mov", "[rbx]", "cl");
					this->emit("ASM", "dec", "rax");
					this->emit("ASM", "inc", "rbx");
					this->emit("ASM", "cmp", "rax", "rbx");
					this->emit("ASM", "jge", "&__RESERVED_INNER_LOOP__");

					this->emit("ASM", "mov", "rax", "0x2000004");
					this->emit("ASM", "mov", "rdi", "0x01");
					this->emit("ASM", "mov", "rsi", "rsp");
					this->emit("INTERRUPT");
					break;
			}

			/* FIXME - `print` INTERRUPT */
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
		if (this->_only_ir_) std::cout << op << "\n" << std::endl;
	} else if (this->_only_ir_) {
		if ("ASM" == op) {
			std::cout << std::setw(5) << "->";
			std::cout << std::left << std::setw(10) << dst;
			std::cout << std::left << std::setw(6) << src;
			std::cout << extra;
		} else {
			std::cout << std::left << std::setw(10) <<op;
			if ("" != dst)   std::cout << ", " << dst;
			if ("" != src)   std::cout << ", " << src;
			if ("" != extra) std::cout << ", " << extra;
		}
		std::cout << std::endl;
	} else {
		std::vector<std::string> regs = { USED_REGISTERS };

		/* call the IR emitter */
		IR::emit(op, dst, src, extra);

		if (regs.end() != std::find(regs.begin(), regs.end(), src)) {
			_D(LOG_INFO, "restore register %s", src.c_str());
			this->_alloc_regs_.push_back(src);
		} else if (regs.end() != std::find(regs.begin(), regs.end(), extra)) {
			_D(LOG_INFO, "restore register %s", extra.c_str());
			this->_alloc_regs_.push_back(extra);
		}
	}
}

/* register allocation algo. */
std::string Zerg::regalloc(std::string src) {
	int cnt = 0;
	std::string tmp;

	if (1 ==  sscanf(src.c_str(), __IR_REG_FMT__, &cnt)) {
		if (0 != _alloc_regs_map_.count(src)) {
			/* HACK - Found in cache */
			tmp = _alloc_regs_map_[src];
		} else {
			ALERT(0 == _alloc_regs_.size());
			tmp = _alloc_regs_[0];
			_alloc_regs_.erase(_alloc_regs_.begin());
		}

		_D(LOG_INFO, "re-allocate register - %s -> %s", src.c_str(), tmp.c_str());
		_alloc_regs_map_[src] = tmp;
		return tmp;
	}

	return src;
}
