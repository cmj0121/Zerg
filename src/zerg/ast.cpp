/* Copyright (C) 2017 cmj<cmj@cmj.tw>. All right reserved. */

#include <string>
#include <sstream>
#include "zerg.h"

AST::AST(AST *node, int lineno) : Tree<AST>(node->_node_.first), _node_(node->_node_) {
	_reg_       = node->_reg_;
	_otype_     = node->_otype_;
	_ctype_     = node->_ctype_;
	_lineno_    = lineno;
	_parent_    = NULL;
	_transfer_  = NULL;
	_branch_[0] = NULL;
	_branch_[1] = NULL;
};
AST::AST(std::string token, ZType type, int lineno) : Tree<AST>(token) ,_node_(token, type) {
	_reg_       = 0;
	_otype_     = OBJ_UNKNOWN;
	_ctype_     = CFG_UNKNOWN;
	_lineno_    = lineno;
	_parent_    = NULL;
	_transfer_  = NULL;
	_branch_[0] = NULL;
	_branch_[1] = NULL;
}
AST::AST(ZergToken &token, int lineno) : Tree<AST>(token.first), _node_(token) {
	_reg_       = 0;
	_otype_     = OBJ_UNKNOWN;
	_ctype_     = CFG_UNKNOWN;
	_lineno_    = lineno;
	_parent_    = NULL;
	_transfer_  = NULL;
	_branch_[0] = NULL;
	_branch_[1] = NULL;
}

std::string AST::data(void) {
	char buff[BUFSIZ] = {0};

	if (0 < this->_reg_) {
		snprintf(buff, sizeof(buff), __IR_REG_FMT__, this->_reg_);
	} else if (ZTYPE_FUNCCALL == this->type() || -1 == this->_reg_) {
		snprintf(buff, sizeof(buff), "%s", __IR_SYSCALL_REG__);
	} else if ("" != this->_symb_) {
		snprintf(buff, sizeof(buff), "%s%s", __IR_REFERENCE__, this->_symb_.c_str());
	} else {
		snprintf(buff, sizeof(buff), "%s", this->raw().c_str());
	}
	return buff;
}
std::string AST::label(bool condition) {
	char buff[BUFSIZ] = {0};

	if (NULL == this->_parent_) return this->data();

	switch(this->_parent_->_ctype_) {
		case CFG_BRANCH:
			if (condition)
				snprintf(buff, sizeof(buff), __IR_BRANCH__, this->_parent_->_lineno_);
			else
				snprintf(buff, sizeof(buff), __IR_BRANCH_END__, this->_parent_->_lineno_);
			break;
		case CFG_BLOCK:
		default:
			snprintf(buff, sizeof(buff), "%s", this->data().c_str());
			break;
	}

	return buff;
}
off_t AST::asInt(void) {
	off_t cnt;
	std::stringstream ss;

	ALERT(ZTYPE_NUMBER != this->type());

	if ("0x" == this->raw().substr(0, 2) || "0X" == this->raw().substr(0, 2)) {
		ss << std::hex << this->raw();
	} else {
		ss << this->raw();
	}

	ss >> cnt;
	return cnt;
}

AST* AST::transfer(AST *node) {
	if (NULL == this->_transfer_) {
		this->_transfer_ = node;
		this->_ctype_    = CFG_UNKNOWN == this->_ctype_ ? CFG_BLOCK : this->_ctype_;
		node->_parent_   = this;
	} else {
		this->_transfer_->transfer(node);
	}

	return this;
}
AST* AST::branch(AST *node, AST *sub) {
	AST *cur = NULL;

	ALERT(0 == this->length());

	cur = this->child(0);

	cur->_lineno_    = this->_lineno_;
	cur->_branch_[0] = node;
	cur->_branch_[1] = this == sub ? cur : sub;
	cur->_ctype_     = CFG_BRANCH;
	node->_parent_   = cur;

	if (NULL != sub) sub->_parent_ = cur;

	return cur;
}
