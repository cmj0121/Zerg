/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iostream>
#include <iomanip>
#include "zerg.h"

ZergToken& Zerg::parser(ZergToken &cur, ZergToken prev) {
	char buff[BUFSIZ] = {0};
	CFG *tmp = NULL;

	static CFG* node = NULL;
	static bool shouldIndent = false;

	if (AST_NEWLINE == cur.type() && AST_NEWLINE == prev.type()) {
		_D(LOG_DEBUG, "pass empty line");
		goto END;
	}

	if (NULL == node) {
		node = new CFG(CFG_MAIN);
		this->_root_[CFG_MAIN] = node;
	} else if (true == shouldIndent) {
		switch(cur.type()) {
			case AST_INDENT:
			case AST_NEWLINE:
				break;
			default:
				_D(LOG_CRIT, "syntax error - should indent %s", cur.c_str());
				break;
		}
	}

	_D(LOG_INFO, "parse %s (%X) with %s on L#%d", cur.c_str(), cur.type(),
													prev.c_str(), _lineno_);
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

		case AST_DOT:
			switch(prev.type()) {
				case AST_IDENTIFIER:
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "Only instance has property, not %s", cur.c_str());
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
				case AST_LOG_NOT:
				case AST_BRACKET_CLOSE: case AST_PARENTHESES_CLOSE:
					if (AST_LOG_NOT == prev.type() && AST_EQUAL != cur.type()) {
						_D(LOG_CRIT, "Not Implemented `%s` (0x%X) -> `%s` (0x%X)",
								prev.c_str(), prev.type(), cur.c_str(), cur.type());
					}
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
							std::cerr << *node << std::endl;
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
			switch(prev.type()) {
				case AST_FUNC: case AST_CLASS:
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
				case AST_PARENTHESES_CLOSE: case AST_BRACKET_CLOSE:
					_D(LOG_CRIT, "Syntax error on %s L#%d `%s`",
										this->_src_.c_str(), this->_lineno_, cur.c_str());
					break;
				default:
					node = node->insert(cur);
					break;
			}
			break;
		case AST_TRUE: case AST_FALSE:
		case AST_NUMBER:
			switch(prev.type()) {
				case AST_TRUE: case AST_FALSE:
				case AST_NUMBER:
				case AST_IDENTIFIER:
				case AST_PARENTHESES_CLOSE: case AST_BRACKET_CLOSE:
					_D(LOG_CRIT, "Syntax error on %s L#%d `%s`",
										this->_src_.c_str(), this->_lineno_, cur.c_str());
					break;
				default:
					node = node->insert(cur);
					break;
			}
			break;
		case AST_STRING:
			switch(prev.type()) {
				case AST_NEWLINE:
				case AST_INDENT: case AST_DEDENT:
					/* NOTE - only treated as comments */
					break;
				default:
					_D(LOG_CRIT, "Not Implemented string");
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
		case AST_OBJECT:
			switch(prev.type()) {
				case AST_ASSIGN:
				case AST_PARENTHESES_OPEN:
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "Not Support %s %s", prev.c_str(), cur.c_str());
					break;
			}

		case AST_NOP:
		case AST_BREAK:
		case AST_CONTINUE:
		case AST_RETURN:
		case AST_DELETE:
			node = node->insert(cur);
			break;

		/* new CFG case */
		case AST_FUNC:
			switch(prev.type()) {
				case AST_NEWLINE:
				case AST_INDENT: case AST_DEDENT:
					tmp  = new CFG(".func.unknown");
					node = tmp;
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "function declare need be first token in statement");
					break;
			}
			break;
		case AST_CLASS:
			switch(prev.type()) {
				case AST_NEWLINE:
				case AST_INDENT: case AST_DEDENT:
					tmp  = new CFG(".cls.unknown");
					node = tmp;
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "class declare need be first token in statement");
					break;
			}
			break;
		case AST_COLON:
			while (NULL != node->parent() && NULL != node->parent()->parent()) {
				node = node->parent();
				if (AST_BRACKET_OPEN == node->type()) break;
			}

			switch(node->type()) {
				case AST_IF:
				case AST_ELSE:
				case AST_WHILE:
				case AST_FUNC: case AST_CLASS:
					_D(LOG_DEBUG, "should INDENT");
					shouldIndent = true;
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

			snprintf(buff, sizeof(buff), __IR_LABEL_FALSE__, node->label().c_str());
			tmp  = new CFG(buff);
			node->branch(node->nextCFG(true), tmp);
			node = tmp->insert(cur);
			break;
		case AST_WHILE:
			snprintf(buff, sizeof(buff), ".while.%04d", this->_lineno_);
			if (0 != node->length()) {
				tmp = new CFG(buff);

				node = node->root();
				node->passto(tmp);
				node = tmp;
			} else {
				node->rename(buff);
			}
			node = node->insert(cur);
			break;
		case AST_INDENT:
			shouldIndent = false;
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
				case AST_CLASS:
				case AST_ROOT:
				case AST_ELSE:
					break;
				default:
					_D(LOG_CRIT, "syntax error `%s` when INDENT", node->data().c_str());
					break;
			}
			break;
		case AST_DEDENT:
			node = node->root();

			do {
				if (NULL == node || NULL == node->prev()) {
					/* Back to the main logical */
					if (0 == this->_root_.count(CFG_MAIN)) {
						tmp = new CFG(CFG_MAIN);
						this->_root_[CFG_MAIN] = tmp;
					}
					node = tmp;
					break;
				} else if (true == node->isCondit() && NULL == node->nextCFG(false)) {
					switch(node->child(0)->type()) {
						case AST_IF:
							switch(node->child(0)->length()) {
								case 1:
								case 2:
									snprintf(buff, sizeof(buff), __IR_LABEL_END__,
																	node->label().c_str());
									tmp = new CFG(buff);

									node->nextCFG(true)->passto(tmp);
									node = tmp;
									break;
								default:
									_D(LOG_CRIT, "Not Implemented 0x%zX",
																node->child(0)->length());
									break;
							}
							break;
						case AST_WHILE:
							snprintf(buff, sizeof(buff), __IR_LABEL_FALSE__, node->label().c_str());
							tmp = new CFG(buff);

							node->branch(node->nextCFG(true), tmp);
							node = tmp;
							break;
						case AST_FUNC: case AST_CLASS:
							/* get the main CFG */
							if (0 == this->_root_.count(CFG_MAIN)) {
								node = new CFG(CFG_MAIN);
								this->_root_[CFG_MAIN] = node;
							} else {
								node = this->_root_[CFG_MAIN];
							}
							break;
						default:
							node = node->prev();
							break;
					}

					break;
				} else {
					node = node->prev();
					continue;
				}
			} while (1);

			break;

		/* PAIR TOKEN */
		case AST_PARENTHESES_OPEN:
			switch(prev.type()) {
				case AST_SYSCALL:
				case AST_BUILDIN_BUFFER:
				case AST_IDENTIFIER:
				case AST_OBJECT:
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "%s %s is NOT support", prev.c_str(), cur.c_str());
					break;
			}
			break;
		case AST_PARENTHESES_CLOSE:
			do {
				if (NULL == node->parent()) {
					std::cerr << *node << std::endl;
					_D(LOG_CRIT, "Syntax error - parentheses does NOT pair");
					break;
				} else if (0 == node->length()) {
					node = node->parent();
					continue;
				} else if (AST_PARENTHESES_OPEN == node->child(0)->type()) {
					if (AST_PARENTHESES_CLOSE != node->child(node->length()-1)->type()) {
						/* Found */
						break;
					}
					/* nested */
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
					std::cerr << *node << std::endl;
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
END:
	#if defined(DEBUG_AST) || defined(DEBUG)
		if (NULL != node) {
			AST* tmp = node->root();

			std::cerr << *tmp << std::endl;
		};
	#endif /* DEBUG_AST */

	return cur;
}

