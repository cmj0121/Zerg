/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include "zerg.h"

ZergToken& Zerg::parser(ZergToken &cur, ZergToken &prev) {
	static AST* node = new AST("");
	AST *tmp = NULL;

	switch(cur.type()) {
		case AST_ADD:
		case AST_SUB:
		case AST_MUL:
		case AST_DIV:
		case AST_MOD:
		case AST_LIKE:
			switch(prev.type()) {
				case AST_NEWLINE:
				case AST_NUMBER:
					tmp = new AST(cur);
					node->insert(tmp);
					node = tmp;
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
				case AST_ADD:
				case AST_SUB:
				case AST_MUL:
				case AST_DIV:
				case AST_MOD:
				case AST_LIKE:
					tmp = new AST(cur);
					node->insert(tmp);
					node = tmp;
					break;
				default:
					_D(LOG_CRIT, "Not Implemented `%s` (0x%X) -> `%s` (0x%X)",
							prev.c_str(), prev.type(), cur.c_str(), cur.type());
					break;
			}
			break;
		case AST_NEWLINE:
			node = node->root();
			if (0 != node->length()) {
				/* process */
				std::cout << *node << std::endl;
			}

			delete node;
			node = new AST("");
			break;
		default:
			_D(LOG_CRIT, "Not Implemented `%s` (0x%X) -> `%s` (0x%X)",
							prev.c_str(), prev.type(), cur.c_str(), cur.type());
			break;
	}

	return cur;
}
