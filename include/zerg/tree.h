/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_TREE_H__
#	define __ZERG_TREE_H__

#include <ostream>
#include <vector>
#include "../utils.h"

template<class T>
class Tree {
	public:
		Tree(std::string src) : _src_(src), _parent_(NULL) {};
		virtual ~Tree() {
			for (unsigned int i = 0; i < this->length(); ++i) {
				delete this->_childs_[i];
			}
			 this->_childs_.clear();
		}

		ssize_t length(void) const {
			/* return numbers of the children */
			return this->_childs_.size();
		}
		ssize_t height(void) const {
			/* return the height of the node in this Tree */
			ssize_t ret = 0;
			Tree<T> *cur = this->_parent_;

			while (NULL != cur) {
				cur = cur->parent();
				ret ++;
			}
			return ret;
		}

		T* parent(void) {
			/* return the parent node */
			return this->_parent_;
		}
		T* root(void) {
			T* tmp = this->_parent_;
			/* return the root of the tree */
			if (NULL != tmp && NULL != tmp->parent()) tmp = tmp->parent();
			return NULL == tmp ? (T *)this : tmp;
		}
		T* child(int nr) {
			ALERT(nr >= this->length());

			return this->_childs_[nr];
		}

		void insert(T *dst) {
			/* insert a tree node */
			this->_childs_.push_back(dst);
			dst->_parent_ = (T *)this;
		}
		void remove(void) {
			/* remove the current node in the tree */

			if (NULL == this->_parent_) {
				_D(LOG_INFO, "Need NOT remove this node since it is root");
				return;
			}

			/* remove from parent */
			Tree<T> *parent = (Tree<T> *)this->_parent_;

			for (auto it = parent->_childs_.begin(); it != parent->_childs_.end(); ++it) {
				if (*it == this) {
					parent->_childs_.erase(it);
					break;
				}
			}
		}

		void replace(T *src) {
			int pos = 0;
			std::vector<T *> childs = this->_parent_->_childs_;

			pos = std::find(childs.begin(), childs.end(), (T *)this) - childs.begin();
			this->_parent_->_childs_[pos] = src;
			src->_parent_ = this->_parent_;

			this->_parent_ = NULL;
		}

		friend std::ostream& operator <<(std::ostream &stream, const Tree<T> &src) {
			std::string indent;
			Tree<T> *cur = (Tree<T> *)&src;

			while (NULL != (cur = cur->parent())) {
				indent = (cur->isLastChild() ? "    " : "│   ") + indent;
			}
			stream << indent;

			if (src.isLastChild()) {
				stream << "└── " << src._src_ << std::endl;
			} else {
				stream << "├── " << src._src_ << std::endl;
			}
			for (ssize_t i = 0; i < src.length(); ++i)
				stream << *src._childs_[i];

			return stream;
		}
	private:
		std::string _src_;
		T *_parent_;
		std::vector<T *> _childs_;

		bool isLastChild(void) const {
			Tree<T> *cur = this->_parent_;

			return NULL == cur || this == cur->child(cur->length()-1);
		}
};

class STR_TREE : public Tree<STR_TREE> {
	public:
		STR_TREE(std::string src) : Tree<STR_TREE>(src), _src_(src) {;}

		STR_TREE *find(std::string src) {
			STR_TREE *ret = NULL;

			if (src == this->_src_) {
				return this;
			}

			for (size_t i = 0; i < this->length(); ++i) {
				if (NULL != (ret = this->child(i)->find(src))) {
					return ret;
				}
			}

			return NULL;
		}
		void insert(std::string src) {
			STR_TREE *tmp = new STR_TREE(src);
			return Tree<STR_TREE>::insert(tmp);
		}
	private:
		std::string _src_;
};

#endif /* __ZERG_TREE_H__ */
