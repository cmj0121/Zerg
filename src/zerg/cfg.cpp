/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include "zerg.h"

CFG::CFG(std::string name) : AST(""), _name_(name), _parent_(NULL) {
	this->_refed_   = false;
	this->_branch_  = false;
	this->_next_[0] = NULL;
	this->_next_[1] = NULL;
#ifdef DEBUG
	/* only used on CFG relationship graph */
	this->_map_ = NULL;
#endif /* DEBUG */
}
CFG::~CFG(void) {
#ifdef DEBUG
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
	if (NULL != this->_next_[0] || NULL != this->_next_[1]) {
		_D(LOG_CRIT, "stage already be assigned");
		exit(-1);
	}
	this->_next_[0] = next;
	next->_refed_   = this->_branch_;

	if (next) next->_parent_ = this;
}
void CFG::branch(CFG *truecase, CFG *falsecase) {
	if (NULL != this->_next_[0] || NULL != this->_next_[1]) {
		_D(LOG_CRIT, "stage already be assigned");
		exit(-1);
	}
	this->_next_[0] = truecase;
	this->_condi_ = true;

	truecase->_refed_ = true;
	if (NULL != falsecase) {
		falsecase->_refed_ = true;
		this->_next_[1] = falsecase;
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
std::string CFG::label(void) {
	/* return the CFG label */
	return this->_name_;
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

#ifdef DEBUG
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
			std::cout << std::setw(layout) << std::left;
			if (i == 0) {
				std::cout << "";
				for (ssize_t j = 0; j < len; ++j) {
					std::cout << std::setw(layout) << stages[j]->_name_;
				}
			} else {
				std::cout << stages[i-1]->_name_;
				for (ssize_t j = 0; j < len; ++j) {
					std::cout << std::setw(layout) << _map_[i-1][j];
				}
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
}
#endif /* DEBUG */
