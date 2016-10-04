/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <fstream>
#include <ostream>
#include <iomanip>
#include <map>
#include <set>
#include "zerg.h"

bool Parser::load(std::string src, std::string stmt) {
	std::fstream fs(src);
	std::string line;

	while (std::getline(fs, line)) {
		if ('\0' == line[0] || '#' == line[0]) {
			continue;
		}

		/* process the grammar rule `stmt : rules` */
		size_t pos = 0;
		std::string stmt;
		std::vector<std::string> rule;

		if (std::string::npos == (pos = line.find(':'))) {
			_D(LOG_CRIT, "Not Valid - `%s`", line.c_str());
			exit(-1);
		}

		stmt = strip(line.substr(0, pos));
		rule = split(strip(line.substr(pos+1)), " ");

		if (false == load(stmt, rule)) {
			_D(LOG_CRIT, "Invalid grammar rule - `%s`", line.c_str());
			exit(-1);
		}
	}

	return this->gentable(stmt);
}
bool Parser::load(std::string stmt, TOKENS rule, TOKENS front, TOKENS end) {
	TOKENS sub, tmp;

	for (size_t i = 0; i < rule.size(); ++i) {
		switch(rule[i][0]) {
			case '(':	/* GROUP */
				for (size_t j = rule.size()-1; j > i; --j) {
					if (')' == rule[j][0]) {
						/* update the front, cur, remainder parts */
						tmp = TOKENS(rule.begin(), rule.begin()+i);
						front.insert(front.end(), tmp.begin(), tmp.end());

						sub = TOKENS(rule.begin()+i+1, rule.begin()+j);

						tmp = TOKENS(rule.begin()+j+1, rule.end());
						end.insert(end.end(), tmp.begin(), tmp.end());

						switch(rule[j][rule[j].size()-1]) {
							case ')':	/* GROUP */
								return load(stmt, sub, front, end);
							case '*':	/* ANY - FIXME */
							case '?':	/* ONE or NONE */
								return load(stmt, TOKENS{}, front, end) && load(stmt, sub, front, end);
							default:
								_D(LOG_WARNING, "Not Implemented");
								return false;
						}
					}
				}
				_D(LOG_WARNING, "parentheses are not paired");
				return false;
			case '|':	/* OR */
				{
					TOKENS	sub1(rule.begin(), rule.begin()+i),
							sub2(rule.begin()+i+1, rule.end());

					return load(stmt, sub1, front, end) && load(stmt, sub2, front, end);
				}
			default:
				break;
		}
	}


	TOKENS ret;

	/* concatenate into one rule */
	ret.insert(ret.end(), front.begin(), front.end());
	ret.insert(ret.end(), rule.begin(), rule.end());
	ret.insert(ret.end(), end.begin(), end.end());

	/* save into rule map */
	if (this->_rules_.end() == this->_rules_.find(stmt)) {
		std::vector<TOKENS> cur = { ret };

		this->_rules_[stmt] = cur;
	} else {
		this->_rules_[stmt].push_back(ret);
	}

	return true;
}
bool Parser::gentable(std::string _stmt, ASTType _prev) {
	bool blRet = false;
	size_t _weight = 0;
	std::vector<ASTType> prev;
	std::map<std::string, ASTType> _map_ = {
		{"NEWLINE",		AST_NEWLINE},
		{"INT",			AST_NUMBER},
		{"IDENTIFIER",	AST_IDENTIFIER},
		{"'='",			AST_ASSIGN},
		{"'+'",			AST_ADD},
		{"'-'",			AST_SUB},
		{"'*'",			AST_MUL},
		{"'/'",			AST_DIV},
		{"'%'",			AST_MOD},
	};

	/* check this grammar rule is processed or not */
	for (auto &it : this->_stmt_) {
		if (it.first == _stmt) {
			for (auto p : it.second) {
				if (p == _prev) {
					blRet = true;
					goto END;
				}
			}

			it.second.push_back(_prev);
			break;
		}
		_weight ++;
	}

	/* first-blood */
	if (_weight == this->_stmt_.size()) {
		std::vector<ASTType> tmp = { _prev };
		this->_stmt_.push_back(std::make_pair(_stmt, tmp));
	}

	for (auto rule : this->_rules_[_stmt]) {
		#ifdef DEBUG
			std::string line;

			for (auto it : rule) line += it + " ";
			_D(LOG_CRIT, "grammar : %-8s -> %s", _stmt.c_str(), strip(line).c_str());
		#endif
		int weight = _weight;

		for (auto grammar : rule) {
			if (grammar == _stmt) {
				weight ++;
				break;
			}
		}

		prev.clear();
		prev.push_back(_prev);
		for (auto grammar : rule) {

			if (_stmt == grammar) {
				/* loop */
				for (auto _p : prev)
					this->_table_[_p][_prev] = weight;
			} else if ('a' <= grammar[0] && 'z' >= grammar[0]) {
				/* another grammar */
				for (auto _p: prev)
					gentable(grammar, _p);
				prev = this->_cached_[grammar];
			} else if (_map_.end() != _map_.find(grammar)) {
				/* token */
				ASTType cur = _map_.find(grammar)->second;

				for (auto _p : prev) {
					this->_table_[_p][cur] = weight;
				}
				prev.clear();
				prev.push_back(cur);
			} else {
				_D(LOG_CRIT, "Not define `%s`", grammar.c_str());
				goto END;
			}
		}
		this->_cached_[_stmt].insert(this->_cached_[_stmt].end(), prev.begin(), prev.end());
	}

	blRet = true;
END:
	return blRet;
}

std::string Parser::stmt(int weight) {
	/* return the statement name */
	return this->_stmt_[weight].first;
}
int Parser::weight(ASTType prev, ASTType cur) {
	int weight = -1;

	if (_table_.end() == _table_.find(prev)) {
		_D(LOG_WARNING, "`0x%X` -> 0x%X not found in parsing table", prev, cur);
		goto END;
	} else if (_table_[prev].end() == _table_[prev].find(cur)) {
		_D(LOG_WARNING, "0x%X -> `0x%X` not found in parsing table", prev, cur);
		goto END;
	}

	weight = _table_[prev][cur];
END:
	return weight;
}

std::ostream& operator <<(std::ostream &stream, const Parser &src) {

	stream << "#ifndef __ZERG_PARSING_TABLE_H__" << std::endl;
	stream << "\t#define __ZERG_PARSING_TABLE_H__" << std::endl;

	{	/* original grammar rules */
		stream << "/* GRAMMAR rules\n * " << std::endl;
		for (const auto stmt : src._rules_) {
			for (auto rule : stmt.second) {
				std::string line = "";

				for (auto it : rule) line += it + " ";
				stream << " * "<< std::setw(8) << stmt.first << " -> " << line << "\n";
			}
		}
		stream << " */\n" << std::endl;
	}

	{	/* parsing table with simplify grammar rules */
		std::map<ASTType, std::string> relationship = {
			{AST_UNKNOWN,		"stmt"},
			{AST_NEWLINE,		"NEWLINE"},
			{AST_NUMBER,		"INT"},
			{AST_IDENTIFIER,	"IDENTIFIER"},
			{AST_ADD,			"'+'"},
			{AST_SUB,			"'-'"},
			{AST_MUL,			"'*'"},
			{AST_DIV,			"'/'"},
			{AST_MOD,			"'%'"},
			{AST_ASSIGN,		"'='"},
		};


		stream << "/* Parsing Table\n *" << std::endl;
		for (auto it : relationship) {
			if (it.first != AST_UNKNOWN) {
				stream << std::left << std::setw(12) << it.second;
			} else {
				stream << " * " << std::left << std::setw(12) << " ";
			}
		}

		stream << std::endl;
		for (auto it : relationship) {
			if (src._table_.end() == src._table_.find(it.first)) {
				continue;
			}

			stream << " * " << std::left << std::setw(12) << it.second;
			for (auto cur : relationship) {
				std::map<_ASTType_, int> clm = src._table_.find(it.first)->second;

				if (cur.first == AST_UNKNOWN) {
					continue;
				} else if (clm.end() == clm.find(cur.first)) {
					stream << std::left << std::setw(12) << " ";
				} else {
					stream << std::left << std::setw(12) << clm.find(cur.first)->second;
				}
			}
			stream << std::endl;
		}

		stream << " */\n" << std::endl;
	}

	{
		std::map<ASTType, std::string> relationship = {
		#define DEF(type)	{ type, #type }
			DEF(AST_UNKNOWN),
			DEF(AST_NEWLINE),
			DEF(AST_NUMBER),
			DEF(AST_IDENTIFIER),
			DEF(AST_ADD),
			DEF(AST_SUB),
			DEF(AST_MUL),
			DEF(AST_DIV),
			DEF(AST_MOD),
			DEF(AST_ASSIGN),
		#undef DEF
		};

		stream << "std::map<ASTType, std::map<ASTType, int>> _table_ = {\n\t";
		for (auto row : src._table_) {
			stream << "{\n\t\t" << relationship[row.first] << ", {\n";
			for (auto it : row.second) {
				stream << "\t\t\t{ " << relationship[it.first] << ",\t" <<it.second << "},\n";
			}
			stream << "\t\t},\n\t}, ";
		}
		stream << "\n};\n";
	}

	stream << "#endif /* __ZERG_PARSING_TABLE_H__ */";
	return stream;
}

