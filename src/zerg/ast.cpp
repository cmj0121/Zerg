#include "zerg.h"

AST::AST(std::string src) : Tree<AST>(src), _emitted_(false), _label_(0), _raw_(src) {
	std::vector<std::string> ops = {"+", "-", "*", "/", "<", ">"};

	if (0 == src.size()) {
		_type_ = AST_ROOT;
	} else if ("syscall" == src) {
		_type_ = AST_INTERRUPT;
	} else if ('\'' == src[0] || '\"' == src[0]) {
		_type_ = AST_STRING;
	} else if (0 == src.find("0x")) {
		_type_ = AST_NUMBER;
	} else if ("=" == src) {
		_type_ = AST_ASSIGN;
	} else if (ops.end() != std::find(ops.begin(), ops.end(), src)) {
		_type_ = AST_OPERATORS;
	} else {
		_type_ = AST_IDENTIFIER;
	}
}

void AST::insert(std::string src) {
	AST *node = new AST(src);

	_D(LOG_DEBUG, "AST insert %s", src.c_str());
	return Tree<AST>::insert(node);
}
void AST::setLabel(int nr) {
	/* Set as label */
	_D(LOG_DEBUG, "AST set label %d on %s", nr, this->_raw_.c_str());
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

void AST::setEmitted(void) {
	/* set this AST already emitted */
	this->_emitted_ = true;
}
bool AST::isEmmited(void) {
	/* reply this AST is emitted or not */
	return this->_emitted_;
}
