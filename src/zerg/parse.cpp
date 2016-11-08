/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include "zerg.h"

ZergToken& Zerg::parser(ZergToken &cur, ZergToken &prev) {
	static CFG* node = new CFG(CFG_MAIN);

	_D(LOG_DEBUG, "parse %s (%X) -> %s (%X)",
		prev.c_str(), prev.type(), cur.c_str(), cur.type());
	switch(cur.type()) {
		case AST_NEWLINE:
			node = node->root();
			if (0 != node->length() && 0 == this->_root_.count(CFG_MAIN)) {
				this->_root_[CFG_MAIN] = node;
			}
			break;

		case AST_ADD:
		case AST_SUB:
		case AST_LIKE:
			switch(prev.type()) {
				case AST_NEWLINE:
				case AST_PRINT:
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
		case AST_MUL:
		case AST_DIV:
		case AST_MOD:
			switch(prev.type()) {
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
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "Not Implemented `%s` (0x%X) -> `%s` (0x%X)",
							prev.c_str(), prev.type(), cur.c_str(), cur.type());
					break;
			}
			break;

		case AST_NUMBER:
			switch(prev.type()) {
				case AST_NEWLINE:
				case AST_PRINT:
				case AST_PARENTHESES_OPEN:
				case AST_COMMA:
				case AST_ASSIGN:
				case AST_ADD:
				case AST_SUB:
				case AST_MUL:
				case AST_DIV:
				case AST_MOD:
				case AST_LIKE:
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
				case AST_NEWLINE:
				case AST_PRINT:
				case AST_ASSIGN:
				case AST_ADD:
				case AST_SUB:
				case AST_MUL:
				case AST_DIV:
				case AST_MOD:
				case AST_LIKE:
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "Not Implemented `%s` (0x%X) -> `%s` (0x%X)",
							prev.c_str(), prev.type(), cur.c_str(), cur.type());
					break;
			}
			break;

		case AST_PRINT:
			switch(prev.type()) {
				case AST_NEWLINE:
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "`print` should be the first token in the statement");
					break;
			}
			break;
		case AST_SYSCALL:
			switch (prev.type()) {
				case AST_NEWLINE:
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "`syscall` should be the first token in the statement");
					break;
			}
			break;

		case AST_PARENTHESES_OPEN:
			switch(prev.type()) {
				case AST_SYSCALL:
					node = node->insert(cur);
					break;
				default:
					_D(LOG_CRIT, "`(` should be the first token in the statement");
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
		default:
			_D(LOG_CRIT, "Not Implemented `%s` (0x%X) -> `%s` (0x%X)",
							prev.c_str(), prev.type(), cur.c_str(), cur.type());
			break;
	}

	#ifdef DEBUG
		AST* tmp = node->root();

		std::cout << *tmp << std::endl;
	#endif /* DEBUG */

	return cur;
}
