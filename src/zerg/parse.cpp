/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include "zerg.h"

AST* Zerg::parser(std::vector<ZergToken> tokens) {
	ZergToken prev = "";
	AST *ast = NULL, *cur = NULL;
	static std::vector<AST *> _stack_;

	/* generate the AST via the parsing table */
	_D(LOG_INFO, "parse with #%zu tokens", tokens.size());
	for (auto &token : tokens) {
		int weight = this->weight(prev.type(), token.type());

		if (-1 == weight) {
			_S(tokens);
			delete ast;
			return NULL;
		}
		token.weight(weight);

		AST *node = new AST(token), *prevnode = NULL;

		_D(LOG_INFO, "process `%s`", token.c_str());
		do {
			/* HACK - Special case for pair-token */
			switch(node->type()) {
				case AST_PARENTHESES_OPEN:
					_stack_.push_back(ast);
					ast = node;
					cur = ast;
					goto NEXT_STEP;
				case AST_PARENTHESES_CLOSE:
					cur = _stack_[_stack_.size()-1];
					_stack_.pop_back();

					cur->insert(ast);
					ast = cur;
					ast->insert(node);
					goto NEXT_STEP;
				case AST_COMMA:					/* TRACE TO TOP */
					cur = ast;
					goto NEXT_STEP;
				default:
					break;
			}

			if (NULL == ast) {				/* first node in AST */
				ast = node;
				cur = node;
				break;
			} else if (NULL == cur) {		/* trace on the root of AST */
				node->insert(ast);
				ast = node;
				cur = node;
				break;
			}

			if (cur->weight() < node->weight()) {	/* FOUND */
				switch (node->type()) {
					case AST_ADD: case AST_SUB: case AST_MUL: case AST_DIV:
					case AST_MOD:					/* BINARY OPERATIONS */
						if (NULL == prevnode || 2 > cur->length()) {
							cur->insert(node);
							cur = node;
						} else {
							prevnode->replace(node);
							node->insert(prevnode);
							cur = node;
						}
						break;
					default:
						cur->insert(node);
						cur = node;
						break;
				}
				break;
			}

			/* NOTE - trace to the parent node */
			prevnode = cur;
			cur = cur->parent();
		} while (true);

	NEXT_STEP:
		/* save the current AST node */
		prev = token;
	}

	#ifdef DEBUG
	std::cout << "point -> `" << cur->data() << "`\n";
	std::cout << *ast << std::endl;
	#endif /* DEBUG */


	return ast;
}
