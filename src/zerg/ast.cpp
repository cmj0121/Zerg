#include <assert.h>
#include "zerg.h"

AST::AST(ZergToken src) : Tree<AST>(src), _emitted_(false), _raw_(src) {
	this->_label_ = 0;
	this->_reg_   = 0;
	this->_type_  = src.type();
}

void AST::insert(ZergToken src) {
	AST *node = new AST(src);
	_D(LOG_INFO, "AST insert %s", src.c_str());
	return this->insert(node);
}
void AST::insert(AST *node) {
	assert(node != NULL);
	return Tree<AST>::insert(node);
}
void AST::setLabel(int nr) {
	/* Set as label */
	_D(LOG_DEBUG, "AST set label %d on %s", nr, this->_raw_.c_str());
	this->_label_ = nr;
}
void AST::setReg(int nr) {
	/* set the register */
	this->_reg_ = nr;
}
int AST::getReg(void) {
	/* get the number of register */
	return this->_reg_;
}
int AST::weight(void) {
	/* reply the weight of the node in AST */
	return this->_raw_.weight();
}
ASTType AST::type(void) {
	/* category of the node in AST */
	return this->_type_;
}
std::string AST::data(void) {
	char buff[BUFSIZ] = {0};

	/* formatted data */
	if (0 != this->_label_) {
		snprintf(buff, sizeof(buff), "__ZERG_LABEL_%d__", this->_label_);
		return buff;
	} else if (0 != this->_reg_) {
		/* return the register without limited count */
		snprintf(buff, sizeof(buff), __IR_REG_FMT__, this->_reg_);
		return buff;
	} else if (AST_STRING == this->_type_) {
		/* return the raw string */
		return this->_raw_.substr(1, this->_raw_.size()-2);
	} else {
		return this->_raw_;
	}
}

void AST::setEmitted(void) {
	/* set this AST already emitted */
	this->_emitted_ = true;
}
bool AST::isEmmited(void) {
	/* reply this AST is emitted or not */
	return this->_emitted_;
}

bool operator== (const AST &obj, const char *src) {
	return obj._raw_ == src;
}
