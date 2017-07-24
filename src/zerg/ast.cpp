/* Copyright (C) 2017 cmj<cmj@cmj.tw>. All right reserved. */

#include <string>
#include "zerg.h"

AST::AST(std::string token, ZType type) : Tree<AST>(token) ,_node_(token, type) {
	_reg_   = 0;
	_otype_ = OBJ_UNKNOWN;
}
AST::AST(ZergToken &token) : Tree<AST>(token.first), _node_(token) {
	_reg_   = 0;
	_otype_ = OBJ_UNKNOWN;
}

std::string AST::data(void) {
	char buff[BUFSIZ] = {0};

	if (0 != this->_reg_) {
		snprintf(buff, sizeof(buff), __IR_REG_FMT__, this->_reg_);
	} else if (ZTYPE_FUNCCALL == this->type()) {
		snprintf(buff, sizeof(buff), "%s", __IR_SYSCALL_REG__);
	} else if ("" != this->_symb_) {
		snprintf(buff, sizeof(buff), "%s%s", __IR_REFERENCE__, this->_symb_.c_str());
	} else {
		snprintf(buff, sizeof(buff), "%s", this->raw().c_str());
	}
	return buff;
}
