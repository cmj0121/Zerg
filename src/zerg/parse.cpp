/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include "zerg.h"

ZergToken& Zerg::parser(ZergToken &cur, ZergToken &prev) {
	char buff[BUFSIZ] = {0};
	CFG *tmp = NULL;

	static CFG* node = NULL;

	if (NULL == node) {
		node = new CFG(CFG_MAIN);
		this->_root_[CFG_MAIN] = node;
	}

	_D(LOG_DEBUG, "parse %s (%X)", cur.c_str(), cur.type());
	switch(cur.type()) {
		case AST_NEWLINE:
			switch(prev.type()) {
				case AST_COLON:
					break;
				default:
					node = node->root();
					break;
			}
			break;

		case AST_ADD:    case AST_SUB:     case AST_LIKE:
		case AST_LOG_NOT:
			switch(prev.type()) {
				case AST_TRUE: case AST_FALSE:
				case AST_NEWLINE:
				case AST_INDENT:
				case AST_PRINT:
				case AST_WHILE:
				case AST_IF:
				case AST_ASSIGN:
				case AST_NUMBER:
				case AST_IDENTIFIER:
				case AST_ADD:
				case AST_SUB:
				case AST_LIKE:
				case AST_MUL:
				case AST_DIV:
				case AST_MOD:
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "Not Implemented `%s` (0x%X) -> `%s` (0x%X)",
							prev.c_str(), prev.type(), cur.c_str(), cur.type());
					break;
			}
			break;
		case AST_MUL:    case AST_DIV:     case AST_MOD:
		case AST_LSHT:   case AST_RSHT:
		case AST_BIT_OR: case AST_BIT_AND: case AST_BIT_XOR:
		case AST_LESS:   case AST_LESS_OR_EQUAL:
		case AST_GRATE:  case AST_GRATE_OR_EQUAL:
		case AST_EQUAL:
		case AST_LOG_OR: case AST_LOG_AND: case AST_LOG_XOR:
		case AST_INC:    case AST_DEC:
			switch(prev.type()) {
				case AST_TRUE: case AST_FALSE:
				case AST_NUMBER:
				case AST_IDENTIFIER:
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "Not Implemented `%s` (0x%X) -> `%s` (0x%X)",
							prev.c_str(), prev.type(), cur.c_str(), cur.type());
					break;
			}
			break;

		case AST_COMMA:
			switch(prev.type()) {
				case AST_NUMBER:
				case AST_IDENTIFIER:
					do {
						if (NULL == node->parent()) {
							std::cout << *node << std::endl;
							_D(LOG_CRIT, "Not Implemented");
							break;
						} else if (AST_PARENTHESES_OPEN == node->parent()->child(0)->type()) {
							/* Found */
							break;
						}

						node = node->parent();
					} while (true);
					break;
				default:
					_D(LOG_CRIT, "Not Implemented `%s` (0x%X) -> `%s` (0x%X)",
							prev.c_str(), prev.type(), cur.c_str(), cur.type());
					break;
			}
			break;
		case AST_ASSIGN:
			switch(prev.type()) {
				case AST_IDENTIFIER:
				case AST_BRACKET_CLOSE:
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "Not Implemented `%s` (0x%X) -> `%s` (0x%X)",
							prev.c_str(), prev.type(), cur.c_str(), cur.type());
					break;
			}
			break;

		case AST_IDENTIFIER:
			if (AST_FUNC == prev.type()) {
				/* remove the current node and set into new AST */
				for (auto it : this->_root_) {
					if (it.second == node->root()) {
						_D(LOG_DEBUG, "remove AST %s", it.first.c_str());
						this->_root_.erase(it.first);
						break;
					}
				}

				node->root()->rename(cur.data());
				this->_root_[cur.data()] = node->root();
				node = node->insert(cur);
				break ;
			}
		case AST_TRUE: case AST_FALSE:
		case AST_NUMBER:
			switch(prev.type()) {
				case AST_TRUE: case AST_FALSE:
				case AST_NUMBER:
				case AST_IDENTIFIER:
					_D(LOG_CRIT, "Syntax error on #%d", this->_lineno_);
					break;
				default:
					node = node->insert(cur);
					break;
			}
			break;

		case AST_PRINT:
			switch(prev.type()) {
				case AST_NEWLINE:
				case AST_INDENT:
				case AST_DEDENT:
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "`%s` should be the first token in the statement", cur.c_str());
					break;
			}
			break;
		case AST_SYSCALL:
		case AST_BUILDIN_BUFFER:
			switch (prev.type()) {
				case AST_NEWLINE:
				case AST_ASSIGN:
				case AST_BUILDIN_BUFFER:
				case AST_INDENT: case AST_DEDENT:
					node = node->insert(cur);
					break;
				case AST_PARENTHESES_CLOSE:
					node = node->parent();
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "`%s` should be the first token in the statement %x",
																		cur.c_str(), prev.type());
					break;
			}
			break;
		case AST_FUNC:
			switch(prev.type()) {
				case AST_NEWLINE:
				case AST_INDENT: case AST_DEDENT:
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "function declare need be first token in statement");
					break;
			}
			break;

		case AST_NOP:
		case AST_BREAK:
		case AST_CONTINUE:
		case AST_RETURN:
		case AST_DELETE:
			node = node->insert(cur);
			break;

		/* new CFG case */
		case AST_COLON:
			while (NULL != node->parent() && NULL != node->parent()->parent()) {
				node = node->parent();
				if (AST_BRACKET_OPEN == node->type()) break;
			}

			switch(node->type()) {
				case AST_IF:
				case AST_ELSE:
				case AST_WHILE:
				case AST_FUNC:
				case AST_BRACKET_OPEN:
					break;
				default:
					_D(LOG_CRIT, "`:` is not the valid syntax on `%s`", node->data().c_str());
					break;
			}
			break;
		case AST_IF:
			snprintf(buff, sizeof(buff), ".if.%04d", this->_lineno_);
			tmp = new CFG(buff);

			node = node->root();
			node->passto(tmp);
			node = tmp;
			node = node->insert(cur);
			break;
		case AST_ELSE:
			ALERT(NULL == node->prev() || NULL == node->prev()->child(0));

			tmp  = node;
			node = node->prev();
			node->passto(NULL);
			delete tmp;

			node = node->prev();
			ALERT(AST_IF != node->child(0)->type());

			snprintf(buff, sizeof(buff), "%s_FALSE", node->label().c_str());
			tmp  = new CFG(buff);
			node->branch(node->nextCFG(true), tmp);
			node = tmp->insert(cur);
			break;
		case AST_WHILE:
			snprintf(buff, sizeof(buff), ".while.%04d", this->_lineno_);
			tmp = new CFG(buff);

			node = node->root();
			node->passto(tmp);
			node = tmp;
			node = node->insert(cur);
			break;
		case AST_INDENT:
			switch(node->type()) {
				case AST_IF:
				case AST_WHILE:
					node = node->root();

					ALERT("" == node->label());
					snprintf(buff, sizeof(buff), "%s_TRUE", node->label().c_str());
					tmp = new CFG(buff);

					node->branch(tmp, NULL);
					node = tmp;
					break;
				case AST_FUNC:
					node = node->root();

					ALERT("" == node->label());
					snprintf(buff, sizeof(buff), "%s_FUNC", node->label().c_str());
					tmp = new CFG(buff);

					node->passto(tmp);
					node = tmp;
					break;
				case AST_ROOT:
				case AST_ELSE:
					break;
				default:
					_D(LOG_CRIT, "syntax error `%s`", node->data().c_str());
					break;
			}
			break;
		case AST_DEDENT:
			node = node->root();

			if (NULL == node->root()->prev()) {
				ALERT(0 != this->_root_.count(CFG_MAIN));

				node = new CFG(CFG_MAIN);
				this->_root_[CFG_MAIN] = node;
			} else {
				tmp  = node->prev();

				ALERT(0 == node->prev()->length());
				switch(tmp->child(0)->type()) {
					case AST_IF:
						switch(tmp->child(0)->length()) {
							case 1:
							case 2:
								snprintf(buff, sizeof(buff), "%s_END", tmp->label().c_str());
								tmp = new CFG(buff);
								node->passto(tmp);
								node = tmp;
								break;
							default:
								_D(LOG_CRIT, "Not Implemented 0x%zX", tmp->child(0)->length());
								break;
						}
						break;
					case AST_WHILE:
						snprintf(buff, sizeof(buff), "%s_FALSE", tmp->label().c_str());
						node = new CFG(buff);

						tmp->branch(tmp->nextCFG(true), node);
						break;
					case AST_FUNC:
						if (0 == this->_root_.count(CFG_MAIN)) {
							node = new CFG(CFG_MAIN);
							this->_root_[CFG_MAIN] = node;
						} else {
							node = this->_root_[CFG_MAIN];
						}
						break;
					default:
						_D(LOG_CRIT, "Not Implemented 0X%X", tmp->child(0)->type());
						break;
				}
			}
			break;

		/* PAIR TOKEN */
		case AST_PARENTHESES_OPEN:
			switch(prev.type()) {
				case AST_SYSCALL:
				case AST_BUILDIN_BUFFER:
				case AST_IDENTIFIER:
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "`(` only follow syscall");
					break;
			}
			break;
		case AST_PARENTHESES_CLOSE:
			do {
				if (NULL == node->parent()) {
					std::cout << *node << std::endl;
					_D(LOG_CRIT, "Syntax error - parentheses does NOT pair");
					break;
				} else if (0 == node->length()) {
					node = node->parent();
					continue;
				} else if (AST_PARENTHESES_OPEN == node->child(0)->type()) {
					/* Found */
					break;
				}

				node = node->parent();
			} while (true);

			node = node->insert(cur);
			break;
		case AST_BRACKET_OPEN:
			switch(prev.type()) {
				case AST_IDENTIFIER:
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "`[` should be following identifier");
					break;
			}
			break;
		case AST_BRACKET_CLOSE:
			do {
				if (NULL == node->parent()) {
					std::cout << *node << std::endl;
					_D(LOG_CRIT, "Syntax error - brackets does NOT pair");
					break;
				} else if (0 == node->length()) {
					node = node->parent();
					continue;
				} else if (AST_BRACKET_OPEN == node->child(0)->type()) {
					/* Found */
					break;
				}

				node = node->parent();
			} while (true);

			node = node->insert(cur);
			break;
		case AST_DICT_OPEN:
		case AST_DICT_CLOSE:
			node = node->insert(cur);
			break;

		default:
			_D(LOG_CRIT, "Not Implemented `%s` (0x%X)", cur.c_str(), cur.type());
			break;
	}

	#ifdef DEBUG_AST
		do {
			AST* tmp = node->root();

			std::cout << *tmp << std::endl;
		} while (0);
	#endif /* DEBUG_AST */

	return cur;
}
