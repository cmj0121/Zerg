#include "zerg.h"

AST::AST(std::string src) : Tree<AST>(src), _emitted_(false), _raw_(src) {
	std::vector<std::string> ops = {"+", "-", "*", "/", "<", ">"};

	this->_label_ = 0;
	this->_reg_   = 0;

	if (0 == src.size()) {
		/* root of AST */
		_type_ = AST_ROOT;
	} else if ("syscall" == src) {
		/* low-level system call (interrupt) */
		_type_ = AST_INTERRUPT;
	} else if ("=" == src) {
		/* assignment */
		_type_ = AST_ASSIGN;
	} else if ('\'' == src[0] || '\"' == src[0]) {
		/* NOTE - string which already verified on stage of lexer analysis */
		_type_ = AST_STRING;
	} else if (0 == src.find("0x")) {
		/* FIXME - need more general */
		_type_ = AST_NUMBER;
	} else if (ops.end() != std::find(ops.begin(), ops.end(), src)) {
		/* FIXME - need classified via the grammar result */
		_type_ = AST_OPERATORS;
	} else {
		/* FIXME - not always be the identifier */
		_type_ = AST_IDENTIFIER;
	}
}

void AST::insert(std::string src) {
	AST *node = new AST(src);

	_D(LOG_INFO, "AST insert %s", src.c_str());
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
