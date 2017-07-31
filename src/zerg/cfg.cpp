/* Copyright (C) 2017 cmj<cmj@cmj.tw>. All right reserved. */

#include "zerg.h"

CFG::CFG(AST *node, std::string name) : _ast_(node), _parent_(NULL), _type_(CFG_UNKNOWN) {
	int cnt = 0;
	AST *ast = NULL, *sub = NULL;
	CFG *cfg = NULL, *tmp = NULL;

	_next_[0] = NULL;
	_next_[1] = NULL;
	_name_    = name;

	if (ZTYPE_UNKNOWN == node->type()) {
		_type_ = CFG_BLOCK;
		while (cnt < node->length()) {
			switch(node->child(cnt)->type()) {
				case ZTYPE_CMD_FUNCTION:
				case ZTYPE_CMD_CLASS:
					ast = node->child(cnt);
					tmp = new CFG(ast, ast->child(0)->raw());
					ast->remove();

					this->passto(tmp);
					break;
				case ZTYPE_CMD_IF:
					if (cnt+1 < node->length()) {
						sub = new AST("[ROOT]", ZTYPE_UNKNOWN);

						while(cnt+1 < node->length()) {
							ast = node->child(cnt+1);
							ast->remove();
							sub->insert(ast);
						}
					}

					ast = node->child(cnt);
					ast->remove();
					cfg = new CFG(ast);
					this->passto(cfg);

					if (NULL != sub) {
						tmp = new CFG(sub);
						cfg->passto(tmp);
					}
					break;
				case ZTYPE_CMD_WHILE:
				case ZTYPE_CMD_FOR:
					_D(LOG_CRIT, "Not Implemented");
					break;
				default:
					++cnt;
					break;
			}
		}
	} else if (ZTYPE_CMD_IF == node->type()) {
		_ast_ = node->child(0);
		ast   = node->child(1);
		sub   = 3 == node->length() ? node->child(2) : NULL;

		_ast_->remove();
		ast->remove();
		_next_[0] = new CFG(ast);
		if (NULL != sub) {
			sub->remove();
			_next_[1] = new CFG(sub);
		}
		_type_ = CFG_BRANCH;
	} else {
		_type_ = CFG_BLOCK;
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
	return _next_[condition ? 0 : 1];
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
