#include <assert.h>
#include "zerg.h"

AST::AST(ZergToken src) : Tree<AST>(src), _raw_(src) {
	this->_emitted_     = false;
	this->_syscall_reg_ = false;
	this->_label_       = 0;
	this->_reg_         = 0;
	this->_type_        = src.type();
	this->_index_		= NULL;
	this->_indexsize_	= 0;
	this->_weight_      = this->_raw_.weight();

	switch(this->_type_) {
		case AST_NUMBER:
		case AST_SYSCALL:
			this->_vtype_ = VTYPE_INTEGER;
			break;
		case AST_LESS:   case AST_LESS_OR_EQUAL:
		case AST_GRATE:  case AST_GRATE_OR_EQUAL:
		case AST_EQUAL:
		case AST_LOG_AND: case AST_LOG_OR: case AST_LOG_XOR: case AST_LOG_NOT:
			this->_vtype_ = VTYPE_BOOLEAN;
			break;
		case AST_BUILDIN_BUFFER:
			this->_vtype_ = VTYPE_BUFFER;
			break;
		default:
			this->_vtype_ = VTYPE_UNKNOWN;
			break;
	}
}

AST* AST::insert(ZergToken src) {
	AST *node = new AST(src);

	return this->insert(node);
}
AST* AST::insert(AST *node) {
	AST *cur = this, *tmp = NULL;

	ALERT(node == NULL);

	if (0 != this->length() && AST_CLASS == this->child(0)->type()) {
		_D(LOG_INFO, "special case `cls`");
		return this->child(0)->insert(node);
	}

	_D(LOG_INFO, "AST insert %s on `%s`", node->data().c_str(), cur->data().c_str());
	switch(node->type()) {
		case AST_ADD:    case AST_SUB:     case AST_LIKE:
			if (!IS_ATOM(cur)) {
				/* Set the operator is the SIGN (w:1) */
				_D(LOG_DEBUG, "%s is treated as sign", node->data().c_str());
				node->weight(1);
			}
		case AST_LESS:   case AST_LESS_OR_EQUAL:
		case AST_GRATE:  case AST_GRATE_OR_EQUAL:
		case AST_EQUAL:
			if (AST_EQUAL == node->type() && AST_LOG_NOT == cur->type()) {
				if (0 != cur->length()) {
					cur = cur->child(0);
					cur->replace(node);

					node->insert(cur);
					break;
				}
			}

			if (0 != cur->weight() && cur->weight() < node->weight()) {
				while (NULL != cur->parent() && 0 != cur->parent()->weight()) {
					if (cur->parent()->weight() <= node->weight()) {
						cur = cur->parent();
						continue;
					}
					break;
				}

				_D(LOG_DEBUG, "insert operator on %s", cur->data().c_str());
				switch(cur->type()) {
					case AST_LESS:   case AST_LESS_OR_EQUAL:
					case AST_GRATE:  case AST_GRATE_OR_EQUAL:
						_D(LOG_INFO, "Special Case - concate via `and`");
						ALERT(2 != cur->length());

						tmp = new AST("and");
						cur->replace(tmp);
						tmp->insert(cur);
						cur = tmp;		/* `and` */

						cur->insert(node);
						tmp = new AST(cur->child(0)->child(1)->data());
						cur->child(1)->insert(tmp);
						break;
					default:
						cur->replace(node);
						node->insert(cur);
						break;
				}
				break;
			}

			Tree<AST>::insert(node);
			break;
		case AST_LOG_NOT:
		case AST_MUL:    case AST_DIV:     case AST_MOD:
		case AST_LSHT:   case AST_RSHT:
		case AST_BIT_OR: case AST_BIT_AND: case AST_BIT_XOR:
		case AST_LOG_OR: case AST_LOG_AND: case AST_LOG_XOR:
		case AST_INC:    case AST_DEC:
			if (0 != cur->weight() && cur->weight() < node->weight()) {
				while (NULL != cur->parent() && 0 != cur->parent()->weight()) {
					if (cur->parent()->weight() <= node->weight()) {
						cur = cur->parent();
						continue;
					}
					break;
				}

				_D(LOG_DEBUG, "insert operator on %s", cur->data().c_str());
				cur->replace(node);
				node->insert(cur);
				break;
			}
			Tree<AST>::insert(node);
			break;
		case AST_ASSIGN:
			cur = cur->root();

			ALERT(0 == cur->length());
			switch(cur->child(0)->type()) {
				case AST_FUNC: case AST_CLASS:
					cur = cur->child(0);
					break;
				default:
					break;
			}

			ALERT(0 == cur->length());
			cur = cur->child(cur->length()-1);
			cur->replace(node);
			node->insert(cur);
			break;
		default:
			_D(LOG_DEBUG, "tree insert into `%s`", this->data().c_str());
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
void AST::setReg(AST *src) {
	this->_reg_			= src->_reg_;
	this->_syscall_reg_	= src->_syscall_reg_;
}
void AST::setReg(std::string reg) {
	ALERT(reg != SYSCALL_REG);
	this->_syscall_reg_ = true;
}
int AST::getReg(void) {
	/* get the number of register */
	return this->_reg_;
}
void AST::setIndex(AST *src, int size) {
	switch(this->_type_) {
		case AST_IDENTIFIER:
			this->_index_     = src;
			this->_indexsize_ = size;
			break;
		default:
			_D(LOG_CRIT, "Not Implemented 0x%x", this->_type_);
			break;
	}
}
std::string AST::getIndex(void) {
	std::string tmp;

	tmp = NULL == this->_index_ ? "" : this->_index_->data();
	return tmp;
}
std::string AST::getIndexSize(void) {
	std::string dst = "";

	switch(this->_indexsize_) {
		case 1:
			dst = ZASM_MEM_BYTE;
			break;
		case 2:
			dst = ZASM_MEM_WORD;
			break;
		case 3:
			dst = ZASM_MEM_DWORD;
			break;
		case 4:
			dst = ZASM_MEM_QWORD;
			break;
		default:
			dst = "";
			break;
	}

	return dst;
}
int AST::weight(void) {
	/* reply the weight of the node in AST */
	return this->_weight_;
}
void AST::weight(int src) {
	/* reset the weight of the node in AST */
	this->_weight_ = src;
}
ASTType AST::type(void) {
	/* category of the node in AST */
	return this->_type_;
}
VType AST::vtype(void) {
	/* category of the value type in AST */
	if (VTYPE_UNKNOWN == this->_vtype_ && 0 < this->length()) {
		return this->child(0)->vtype();
	}
	return this->_vtype_;
}
VType AST::vtype(VType src) {
	/* Set the value type in AST */
	_D(LOG_DEBUG, "set v-type `%s` -> 0x%X", this->data().c_str(), src);
	this->_vtype_ = src;
	return this->_vtype_;
}
std::string AST::data(void) {
	char buff[BUFSIZ] = {0};

	/* formatted data */
	if (0 != this->_label_) {
		snprintf(buff, sizeof(buff), __IR_LABEL_FMT__, this->_label_);
		return buff;
	} else if (true == this->_syscall_reg_) {
		snprintf(buff, sizeof(buff), "%s", __IR_SYSCALL_REG__);
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
std::string AST::raw(void) {
	return this->_raw_;
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
