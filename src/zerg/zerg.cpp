/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include "zerg.h"


Zerg::Zerg(std::string src, std::string dst, off_t entry) : IR(dst, entry) {
	this->_labelcnt_ = 0;
	this->_root_     = new AST();

	AST *root = this->_root_;

	/* syscall (0x2000004, 0x02, "ZasmP IR\n", 0x0C) */
	root->insert("syscall");
	root->child(0)->insert("0x2000004");
	root->child(0)->insert("0x01");
	root->child(0)->insert("'ZasmP IR\\n'");
	root->child(0)->insert("0x0C");

	/* syscall (0x2000001, 0x04) */
	root->insert("syscall");
	root->child(1)->insert("0x2000001");
	root->child(1)->insert("0x04");

	std::cout << (*root) << std::endl;
	std::cout << "== DFS ==\n";
	this->DFS();
}
Zerg::~Zerg() {
	delete this->_root_;
}

void Zerg::lexer(void) {
	_D(LOG_CRIT, "Not Implemented");
	exit(-1);
}

void Zerg::parser(void) {
	_D(LOG_CRIT, "Not Implemented");
	exit(-1);
}

void Zerg::DFS(AST *node) {
	if (NULL == node) {
		this->DFS(this->_root_);
		for (auto it : this->_symb_) {
			IR::emit("LABEL", it.first, it.second);
		}
	} else {
		/* main DFS logical */
		for (ssize_t i = 0; i < node->length(); ++i) {
			this->DFS(node->child(i));
		}
		this->compile(node);
	}
}
void Zerg::compile(AST *node) {
	std::string tmp;

	/* translate AST to IR */
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
		case AST_INTERRUPT:
			for (ssize_t i = 0; i < node->length(); ++i) {
				AST *child = node->child(i);

				switch(child->type()) {
					case AST_STRING:
						IR::emit("PARAM", "&" + node->child(i)->data());
						break;
					default:
						IR::emit("PARAM", node->child(i)->data());
						break;
				}
			}
			IR::emit("INTERRUPT");
			break;
		default:
			_D(LOG_BUG, "Not implemented %X", node->type());
			break;
	}
}
