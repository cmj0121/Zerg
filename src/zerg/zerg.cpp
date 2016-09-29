/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include "zerg.h"

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

	this->parser();

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

/* Private Properties */
void Zerg::lexer(void) {
	_D(LOG_CRIT, "Not Implemented");
	exit(-1);
}
void Zerg::parser(void) {
	AST *cur = NULL;
	std::vector<ZergToken> tokens[] = {
		{"_VAR", "=", "0"},
		{"_VAR", "=", "_VAR", "+", "1", "*", "-", "2", "%", "3", "/", "-", "-", "4"},
	};

	for (auto token: tokens) {
		if (NULL != (cur = this->parser(token))) {
			std::cout << *cur << std::endl;
			delete cur;
		}
	}
}
AST* Zerg::parser(std::vector<ZergToken> tokens) {
	ZergToken prev = "";
	AST *ast = NULL, *cur = NULL;

	std::vector<std::string> grammar = {"assign", "expr", "pow", "atom"};
	std::map<ASTType, std::map<ASTType, int>> _map_ = {
		{
			AST_UNKNOWN, {
				{AST_NEWLINE,		0},
				{AST_NUMBER,		4},
				{AST_IDENTIFIER,	4},
				{AST_ADD,			5},
				{AST_SUB,			5},
			}
		}, {
			AST_NUMBER, {
				{AST_NEWLINE,	0},
				{AST_ADD,		2},
				{AST_SUB,		2},
				{AST_MUL,		3},
				{AST_DIV,		3},
				{AST_MOD,		3},
			}
		}, {
			AST_IDENTIFIER, {
				{AST_NEWLINE,	0},
				{AST_ADD,		2},
				{AST_SUB,		2},
				{AST_MUL,		3},
				{AST_DIV,		3},
				{AST_MOD,		3},
				{AST_ASSIGN,	1},
			}
		}, {
			AST_ADD, {
				{AST_NUMBER,		4},
				{AST_IDENTIFIER,	4},
				{AST_ADD,			5},
				{AST_SUB,			5},
			}
		}, {
			AST_SUB, {
				{AST_NUMBER,		4},
				{AST_IDENTIFIER,	4},
				{AST_ADD,			5},
				{AST_SUB,			5},
			}
		}, {
			AST_MUL, {
				{AST_NUMBER,		4},
				{AST_IDENTIFIER,	4},
				{AST_ADD,			5},
				{AST_SUB,			5},
			}
		}, {
			AST_DIV, {
				{AST_NUMBER,		4},
				{AST_IDENTIFIER,	4},
				{AST_ADD,			5},
				{AST_SUB,			5},
			}
		}, {
			AST_MOD, {
				{AST_NUMBER,		4},
				{AST_IDENTIFIER,	4},
				{AST_ADD,			5},
				{AST_SUB,			5},
			}
		}, {
			AST_ASSIGN, {
				{AST_NUMBER,		4},
				{AST_IDENTIFIER,	4},
				{AST_ADD,			5},
				{AST_SUB,			5},
			}
		}
	};

	/* generate the AST via the parsing table */
	for (auto &token : tokens) {
		/* syntax check - check the source code is valid or not */
		if (_map_.end() == _map_.find(prev.type())) {
			std::string line;

			for (auto &it : tokens) line += it + " ";
			_D(LOG_CRIT, "Syntax Error `%s` on %s", token.c_str(), line.c_str());
			delete ast;
			return NULL;
		} else if (_map_[prev.type()].end() == _map_[prev.type()].find(token.type())) {
			std::string line;

			for (auto &it : tokens) line += it + " ";
			_D(LOG_CRIT, "Syntax Error `%s` on %s", token.c_str(), line.c_str());
			return NULL;
		}
		token.weight(_map_[prev.type()][token.type()]);

		_D(LOG_DEBUG, "grammar (W:%d) `%s` -> %s %s",
					token.weight(),
					grammar[_map_[prev.type()][token.type()]].c_str(),
					prev.c_str(),
					token.c_str());

		AST *tmp = new AST(token), *prevnode = NULL;
		do {
			if (NULL == ast) {							/* first node in AST */
				ast = tmp;
				cur = ast;
				break;
			} else if (NULL == cur) {					/* trace on the root of AST */
				tmp->insert(ast);
				ast = tmp;
				cur = ast;
				goto NEXT_STEP;
			} else if(cur->weight() < tmp->weight()) {	/* found */
				if (NULL == prevnode || 2 > cur->length()) {
					cur->insert(tmp);
					cur = tmp;
					goto NEXT_STEP;
				} else {
					switch(cur->type()) {
						default:
							prevnode->replace(tmp);
							tmp->insert(prevnode);
							cur = tmp;
							goto NEXT_STEP;
							break;
					}
					break;
				}
				cur->insert(tmp);
				cur = tmp;
NEXT_STEP:
				break;
			}

			prevnode = cur;
			cur = cur->parent();
		} while (true);

		#ifdef DEBUG
			std::cout << "point -> `" << cur->data() << "`\n";
			std::cout << *ast << std::endl;
		#endif /* DEBUG */
		prev = token;
	}

	return ast;
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
		this->DFS(node);

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
void Zerg::DFS(AST *node) {
	/* main DFS logical */
	for (ssize_t i = 0; i < node->length(); ++i) {
		this->DFS(node->child(i));
	}
	this->emitIR(node);
}
void Zerg::emitIR(AST *node) {
	static int regs = 0;
	static std::vector<std::string> stack;
	std::string tmp;

	/* translate AST to IR */
	_D(LOG_INFO, "emit IR on %s", node->data().c_str());
	switch(node->type()) {
		case AST_ROOT:
		case AST_NUMBER:
			/* NOP */
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
					do {
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
					} while (0);
					break;
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
		case AST_RESERVED:
			if (*node == "syscall") {
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
			}
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
