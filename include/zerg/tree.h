/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_TREE_H__
#	define __ZERG_TREE_H__

template<class T>
class Tree {
	public:
		Tree(std::string src) : _src_(src), _parent_(NULL) {};
		virtual ~Tree() {
			for (unsigned int i = 0; i < this->length(); ++i) {
				delete this->_childs_[i];
			}
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

		Tree<T>* parent(void) {
			/* return the parent node */
			return this->_parent_;
		}
		T* child(int nr) {
			if (nr >= this->length()) {
				_D(LOG_WARNING, "Out of boundary %d", nr);
				return NULL;
			}
			return this->_childs_[nr];
		}

		void insert(T *dst) {
			/* insert a tree node */
			this->_childs_.push_back(dst);
			dst->_parent_ = this;
		}
		void remove(void) {
			/* remove the current node in the tree */

			if (NULL == this->_parent_) {
				_D(LOG_INFO, "Need NOT remove this node since it is root");
				return;
			}

			/* remove from parent */
			for (auto it = this->_parent_->_childs_.begin(); it != this->_parent_->_childs_.end(); ++it) {
				if (*it == this) {
					this->_parent_->_childs_.erase(it);
					break;
				}
			}
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
		Tree<T> *_parent_;
		std::vector<T *> _childs_;

		bool isLastChild(void) const {
			Tree<T> *cur = this->_parent_;

			return NULL == cur || this == cur->child(cur->length()-1);
		}
};

#endif /* __ZERG_TREE_H__ */
