/* Copyright (C) 2017 cmj<cmj@cmj.tw>. All right reserved. */
#ifndef __ZERG_CFG__
#  define __ZERG_CFG__

typedef enum _tag_cfg_type_ {
	CFG_UNKNOWN	= 0x0,
	CFG_BLOCK,
	CFG_BRANCH,
} CFGType;

class CFG {
	public:
		CFG(AST *node, std::string name="");
		std::string name(void);

		CFGType type(void);
		AST *ast(void);

		CFG *transfer(void);
		CFG *branch(bool condition);
		CFG *parent(void);

		void passto(CFG *node);
	private:
		std::string _name_;
		AST *_ast_;
		CFG *_next_[2], *_parent_;
		CFGType _type_;
};

#endif /* __ZERG_CFG__ */

