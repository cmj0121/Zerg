#include "zerg.h"

AST::AST(std::string src) : Tree<AST>(src), _label_(0), _raw_(src) {
	if (0 == src.size()) {
		_type_ = AST_ROOT;
	} else if ("syscall" == src) {
		_type_ = AST_INTERRUPT;
	} else if ('\'' == src[0] || '\"' == src[0]) {
		_type_ = AST_STRING;
	} else {
		_type_ = AST_NUMBER;
	}
}

void AST::insert(std::string src) {
	AST *node = new AST(src);
	return Tree<AST>::insert(node);
}
void AST::setLabel(int nr) {
	/* Set as label */
	this->_label_ = nr;
}
ASTType AST::type(void) {
	/* category of the node in AST */
	return this->_type_;
}
std::string AST::data(void) {
	/* formatted data */
	if (0 != this->_label_) {
		char buff[BUFSIZ] = {0};

		snprintf(buff, sizeof(buff), "ZERG_LABEL_%d", this->_label_);
		return buff;
	} else if (AST_STRING == this->_type_) {
		return this->_raw_.substr(1, this->_raw_.size()-2);
	} else {
		return this->_raw_;
	}
}
