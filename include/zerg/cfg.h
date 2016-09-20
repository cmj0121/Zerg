/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_CFG_H__
#	define __ZERG_CFG_H__

#include <vector>

#include "ast.h"
#define CFG_MAIN	"__MAIN__"

/* CFG - Control Flow Graph
 *
 * The graph used to describe the control flow (condition and repeat)
 * of the AST, each node means a AST and the edge is the relation between
 * each AST:
 *
 *     1- Always pass-to
 *     2- Boolean case, i.e. True and False
 */
class CFG : public AST {
	public:
		CFG(std::string name);
		virtual ~CFG();

		bool isRefed(void);
		bool isBranch(void);
		bool isCondit(void);
		void passto(CFG *next);
		void branch(CFG *truecase, CFG *falsecase);
		std::string label(void);

		CFG *nextCFG(bool branch=true);

		friend std::ostream& operator <<(std::ostream &stream, const CFG &src);
	private:
		bool _refed_, _condi_, _branch_;
		std::string _name_;
		CFG *_next_[2], *_parent_;
#ifdef DEBUG
		/* CFG relation map */
		std::vector<CFG *>stages;
		bool **_map_;

		void buildRelation(CFG *node);
#endif /* DEBUG */
};
#endif /* __ZERG_CFG_H__ */
