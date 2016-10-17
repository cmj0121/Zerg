/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_PARSING_TABLE_H__
#	define __ZERG_PARSING_TABLE_H__

typedef std::vector<std::string> TOKENS;


class ParsingTable {
	public:
		ParsingTable(void);

		bool load(std::string src, std::string stmt="stmt");
		bool load(std::string stmt, TOKENS rule, TOKENS front, TOKENS ends);
		void gentable(std::string stmt, ASTType type);

		int weight(ASTType prev, ASTType cur);

		friend std::ostream& operator <<(std::ostream &stream, const ParsingTable &src);
	private:
		STR_TREE *_stmt_;
		std::vector<std::pair<std::string, ASTType>> _processed_;
		std::map<std::string, std::vector<TOKENS>>   _rules_;
		std::map<std::string, std::vector<ASTType>>  _cache_;

		#include "zerg/parse_inc.h"
};

#endif /* __ZERG_PARSING_TABLE_H__ */
