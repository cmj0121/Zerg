/* Copyright (C) 2017 cmj<cmj@cmj.tw>. All right reserved. */

#include "zerg.h"

CFG::CFG(AST *node, std::string name) : _ast_(node), _parent_(NULL), _type_(CFG_UNKNOWN) {
	_next_[0] = NULL;
	_next_[1] = NULL;
	_name_    = name;
	_type_ = CFG_BLOCK;

	if (ZTYPE_UNKNOWN == node->type()) {
		int cnt = 0;
		AST *ast = NULL;
		CFG *tmp = NULL;

		while (cnt < node->length()) {
			switch(node->child(cnt)->type()) {
				case ZTYPE_CMD_FUNCTION:
				case ZTYPE_CMD_CLASS:
					ast = node->child(cnt);
					tmp = new CFG(ast, ast->child(0)->raw());
					ast->remove();

					this->passto(tmp);
					break;
				default:
					++cnt;
					break;
			}
		}
	}
}

std::string CFG::name(void) {
	return this->_name_;
}

CFGType CFG::type(void) {
	return this->_type_;
}

AST* CFG::ast(void) {
	return this->_ast_;
}

CFG* CFG::transfer(void) {
	ALERT(_type_ != CFG_BLOCK);
	return _next_[0];
}

CFG* CFG::branch(bool condition) {
	ALERT(_type_ != CFG_BRANCH);
	return _next_[condition ? 1 : 0];
}
CFG* CFG::parent(void) {
	return this->_parent_;
}

void CFG::passto(CFG *node) {
	switch(this->_type_) {
		case CFG_BLOCK:
			if (NULL == _next_[0]) _next_[0] = node;
			else _next_[0]->passto(node);
			break;
		case CFG_BRANCH:
			if (NULL != _next_[0]) _next_[0]->passto(node);
			if (NULL != _next_[1]) _next_[1]->passto(node);
			break;
		default:
			_D(LOG_CRIT, "Not Support %d", this->_type_);
			break;
	}
}
