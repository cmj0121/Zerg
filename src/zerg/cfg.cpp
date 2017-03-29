/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iostream>
#include <iomanip>
#include "zerg.h"

CFG::CFG(std::string name) : AST(""), _name_(name), _parent_(NULL) {
	this->_refed_   = false;
	this->_branch_  = false;
	this->_condi_   = false;
	this->_next_[0] = NULL;
	this->_next_[1] = NULL;

	#if defined(DEBUG_CFG) || defined(DEBUG)
		/* only used on CFG relationship graph */
		this->_map_ = NULL;
	#endif /* DEBUG */
}
CFG::~CFG(void) {
	#if defined(DEBUG_CFG) || defined(DEBUG)
	if (NULL != this->_map_) {
		for (unsigned int i = 0; i < this->stages.size(); ++i) {
			delete this->_map_[i];
		}
		delete this->_map_;
	}
	#endif /* DEBUG */
}

bool CFG::isRefed(void) {
	/* reply the node is referred or not */
	return this->_refed_;
}
bool CFG::isBranch(void) {
	/* reply the branch node or not */
	return this->_branch_;
}
bool CFG::isCondit(void) {
	/* reply the condition node or not */
	return this->_condi_;
}
void CFG::passto(CFG *next) {
	this->_next_[0] = next;

	if (next) {
		next->_refed_   = this->_branch_;
		next->_parent_ = this;
	}
}
void CFG::branch(CFG *truecase, CFG *falsecase) {
	ALERT(NULL != this->_next_[0] && NULL != truecase  && this->_next_[0] != truecase);
	ALERT(NULL != this->_next_[1] && NULL != falsecase && this->_next_[1] != falsecase);

	this->_next_[0] = truecase;
	this->_next_[1] = falsecase;
	this->_condi_ = true;

	truecase->_refed_ = true;
	if (NULL != falsecase) {
		falsecase->_refed_ = true;
	}

	/* set the branch node */
	if (NULL != truecase) {
		truecase->_branch_  = true;
		truecase->_parent_  = this;
	}
	if (NULL != falsecase) {
		falsecase->_branch_ = true;
		falsecase->_parent_ = this;
	}
}
void CFG::rename(std::string name) {
	/* rename the label */
	this->_name_ = name;
}
std::string CFG::label(void) {
	/* return the CFG label */
	return this->_name_;
}

std::string CFG::varcnt(void) {
	int cnt = 0;
	char cntvar[64] = {0};
	CFG *root = this;

	while (NULL != root->prev()) {
		root = root->prev();
	}

	cnt = root->_varcnt_();
	if (0 != cnt) {
		snprintf(cntvar, sizeof(cntvar), "0x%X", cnt * PARAM_SIZE);
	}

	_D(LOG_DEBUG, "variable count on %s - %d", root->label().c_str(), cnt);
	return cntvar;
}
size_t CFG::_varcnt_(void) {
	size_t cnt = 0;

	for (ssize_t i = 0; i < this->length(); ++i) {
		AST *child = (AST *)this->child(i);

		switch(child->type()) {
			case AST_ASSIGN:
				if (0 != child->length() && 0 == child->child(0)->length()) {
					cnt ++;
				}
				break;
			case AST_FUNC:
				if (0 != child->length() && 0 != child->child(0)->length()) {
					cnt += child->child(0)->child(0)->length();
				}
				break;
			default:
				_D(LOG_DEBUG, "Need NOT count on %s", child->data().c_str());
				break;
		}
	}

	cnt += (NULL == this->nextCFG(true)  ? 0 : this->nextCFG(true)->_varcnt_());
	cnt += (NULL == this->nextCFG(false) ? 0 : this->nextCFG(false)->_varcnt_());

	return cnt;
}

CFG* CFG::insert(ZergToken dst) {
	CFG *tmp = (CFG *)AST::insert(dst);
	return tmp;
}
CFG* CFG::parent(void) {
	CFG* tmp = (CFG *)AST::parent();
	return tmp;
}
CFG* CFG::prev(void) {
	CFG* tmp = this->_parent_;
	return tmp;
}
CFG* CFG::root(void) {
	CFG *tmp = (CFG *)AST::root();
	return tmp;
}
CFG* CFG::nextCFG(bool branch) {
	/* return the next CFG stage */
	return this->_next_[branch ? 0 : 1];
}
std::ostream& operator <<(std::ostream &stream, const CFG &src) {
	AST *node = (AST *)&src;

	stream << "\n\n==== CFG stage (" << src._name_ << ") ====" << std::endl;
	stream << *node;
	stream << "================";

	return stream;
}

#if defined(DEBUG_CFG) || defined(DEBUG)
/* Build the CFG relationship as 2D matrix */
void CFG::buildRelation(CFG *node) {
	/* First, get all stages */
	if (NULL != node && stages.end() == std::find(stages.begin(), stages.end(), node)) {
		stages.push_back(node);
		buildRelation(node->nextCFG(true));
		buildRelation(node->nextCFG(false));
	}

	/* Build the map */
	if (this == node) {
		/* map construction */
		this->_map_ = new bool*[this->stages.size()];
		for (ssize_t i = 0; i < this->stages.size(); ++i) {
			this->_map_[i] = new bool[this->stages.size()];
			for (ssize_t j = 0; j < this->stages.size(); ++j) {
				this->_map_[i][j] = false;
			}
		}


		for (auto st : this->stages) {
			int from, to;
			CFG *cur = NULL;

			from = std::find(stages.begin(), stages.end(), st) - stages.begin();
			if (NULL != (cur = st->nextCFG(true))) {
				to = std::find(stages.begin(), stages.end(), cur) - stages.begin();

				this->_map_[from][to] = true;
				_D(LOG_INFO, "%d -> %d", from, to);
			}

			if (NULL != (cur = st->nextCFG(false))) {
				to = std::find(stages.begin(), stages.end(), cur) - stages.begin();

				this->_map_[from][to] = true;
				_D(LOG_INFO, "%d -> %d", from, to);
			}
		}

		int layout = 10;
		ssize_t len = this->stages.size();

		for (ssize_t i = 0; i < len+1; ++i) {
			std::cerr << std::setw(layout) << std::left;
			if (i == 0) {
				std::cerr << "";
				for (ssize_t j = 0; j < len; ++j) {
					std::cerr << std::setw(layout) << stages[j]->_name_;
				}
			} else {
				std::cerr << stages[i-1]->_name_;
				for (ssize_t j = 0; j < len; ++j) {
					std::cerr << std::setw(layout) << _map_[i-1][j];
				}
			}
			std::cerr << std::endl;
		}
		std::cerr << std::endl;
	}
}
#endif /* DEBUG */
