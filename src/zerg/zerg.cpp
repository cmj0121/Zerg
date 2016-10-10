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

/* Private Properties */
void Zerg::lexer(std::string src) {
	std::string line;
	std::fstream fs(src);
	AST *ast = NULL;
	CFG *cfg = NULL, *curcfg = NULL;

	if (!fs.is_open()) {
		/* cannot open the source code */
		_D(LOG_CRIT, "source file `%s` does NOT exist", src.c_str());
	}

	while (std::getline(fs, line)) {
		TOKENS _tokens;

		for (size_t cur = 0; cur <= line.size(); ++cur) {
			size_t pos;
			_D(LOG_DEBUG, "read line `%s` on %zu #%zu", line.c_str(), cur, _tokens.size());

			switch(line[cur]) {
				case '\0':				/* NEWLINE */
				case '#' :				/* COMMENT */
					goto NEXT_STEP;
					break;
				case ' ': case '\t':	/* SPACE */
					/* FIXME - Maybe INDENT or DEDENT */
					break;
				case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': case '8':
				case '9': case '0':
					for (pos = cur; pos <= line.size(); ++pos) {
						if (line[pos] >= '0' && line[pos] <= '9') {
							/* NUMBER */
							continue;
						}
						break;
					}
					_tokens.push_back(line.substr(cur, pos-cur));
					cur = pos - 1;
					break;
				case '+': case '-':						/* OPERATOR */
				case '*': case '/': case '%': case '~':	/* OPERATOR */
				case '.': case ',': case ':':
				case '(': case ')': case '[': case ']':
				case '{': case '}':
					_tokens.push_back(line.substr(cur, 1));
					break;
				case '\'': case '"':					/* STRING */
					for (pos = cur+1; pos <= line.size(); ++pos) {
						if (line[pos] == line[cur]) break;
					}

					if (line[pos] != line[cur]) {
						_D(LOG_CRIT, "Invalid syntax for string %s", line.c_str());
					}
					_tokens.push_back(line.substr(cur, pos-cur+1));
					cur = pos;
					break;
				default:
					for (pos = cur+1; pos <= line.size(); ++pos) {
						if ((line[pos] | 0x20) >= 'a' && (line[pos] | 0x20) <= 'z') {
							/* Alphabet */
							continue;
						} else if (line[pos] >= '0' && line[pos] <= '9') {
							/* NUMBER */
							continue;
						} else if (line[pos] == '_') {
							/* UNDERLINE */
							continue;
						}
						break;
					}
					_tokens.push_back(line.substr(cur, pos-cur));
					cur = pos - 1;
					break;
			}
		}

		NEXT_STEP:
			std::vector<ZergToken> tokens;

			if (0 != _tokens.size()) {
				for (auto it : _tokens) {
					ZergToken tmp(it);

					tokens.push_back(tmp);
				}

				ALERT(NULL == (ast = this->parser(tokens)));

				cfg = new CFG("");
				cfg->insert(ast);

				if (0 == this->_root_.size()) {
					this->_root_[CFG_MAIN] = cfg;
					curcfg = cfg;
				} else {
					curcfg->passto(cfg);
					curcfg = cfg;
				}
			}
	}
}
AST* Zerg::parser(std::vector<ZergToken> tokens) {
	ZergToken prev = "";
	AST *ast = NULL, *cur = NULL;

	/* generate the AST via the parsing table */
	_D(LOG_INFO, "parse with #%zu tokens", tokens.size());
	for (auto &token : tokens) {
		int weight = this->weight(prev.type(), token.type());

		if (-1 == weight) {
			std::string line;

			for (auto &it : tokens) line += it + " ";
			_D(LOG_CRIT, "Syntax Error `%s` on %s", token.c_str(), line.c_str());
			delete ast;
			return NULL;
		}
		token.weight(weight);

		_D(LOG_DEBUG, "grammar (W:%d) `%s` -> %s %s",
					token.weight(),
					this->stmt(weight).c_str(),
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
	AST *x = NULL, *y = NULL;


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
