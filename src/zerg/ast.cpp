/* Copyright (C) 2017 cmj<cmj@cmj.tw>. All right reserved. */

#include <string>
#include "zerg.h"

AST::AST(std::string token, ZType type) : Tree<AST>(token) ,_node_(token, type) {
	_reg_ = 0;
}
AST::AST(ZergToken &token) : Tree<AST>(token.first), _node_(token) {
	_reg_ = 0;
}

std::string AST::data(void) {
	std::string tmp;
	char buff[BUFSIZ] = {0};

	if (0 != this->_reg_) {
		snprintf(buff, sizeof(buff), __IR_REG_FMT__, this->_reg_);
		tmp = buff;
	} else {
		tmp = this->raw();
		_D(LOG_CRIT, "Not Improvement");
	}
	return buff;
}
