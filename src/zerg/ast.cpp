#include <assert.h>
#include "zerg.h"

AST::AST(ZergToken src) : Tree<AST>(src), _emitted_(false), _raw_(src) {
	this->_label_ = 0;
	this->_reg_   = 0;
	this->_type_  = src.type();
}

AST* AST::insert(ZergToken src) {
	AST *node = new AST(src);

	return this->insert(node);
}
AST* AST::insert(AST *node) {
	AST *cur = this;

	ALERT(node == NULL);

	_D(LOG_INFO, "AST insert %s", node->data().c_str());
	switch(node->type()) {
		case AST_ADD:
		case AST_SUB:
		case AST_MUL:
		case AST_DIV:
		case AST_MOD:
		case AST_LIKE:
			if (0 != cur->weight() && cur->weight() < node->weight()) {
				while (NULL != cur->parent() && 0 != cur->parent()->weight()) {
					if (cur->parent()->weight() <= node->weight()) {
						cur = cur->parent();
						continue;
					}
					break;
				}

				cur->replace(node);
				node->insert(cur);
				break;
			}
			Tree<AST>::insert(node);
			break;
		case AST_ASSIGN:
			cur = cur->root();

			ALERT(0 == cur->length());
			cur = cur->child(cur->length()-1);
			cur->replace(node);
			node->insert(cur);
			break;
		default:
			Tree<AST>::insert(node);
			break;
	}

	return node;
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
		snprintf(buff, sizeof(buff), __IR_LABEL_FMT__, this->_label_);
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
