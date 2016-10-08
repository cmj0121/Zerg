/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <fstream>
#include <ostream>
#include <iomanip>
#include <map>
#include <set>
#include "zerg.h"


static std::map<ASTType, std::pair<std::string, std::string>> _relation_ = {
	#define DEF(type, ir)	{ type, { #type, ir } }
		DEF(AST_UNKNOWN,	""),
		DEF(AST_ROOT,		"stmt"),

		DEF(AST_NEWLINE,	"NEWLINE"),
		DEF(AST_INDENT,		"INDENT"),
		DEF(AST_DEDENT,		"DEDENT"),

		DEF(AST_NUMBER,		"NUMBER"),
		DEF(AST_STRING,		"STRING"),
		DEF(AST_IDENTIFIER,	"IDENTIFIER"),

		DEF(AST_PARENTHESES_OPEN,	"'('"),
		DEF(AST_PARENTHESES_CLOSE,	"')'"),

		/* operations */
		DEF(AST_COLON,		"':'"),
		DEF(AST_COMMA,		"','"),
		DEF(AST_ASSIGN,		"'='"),
		DEF(AST_ADD,		"'+'"),
		DEF(AST_SUB,		"'-'"),
		DEF(AST_MUL,		"'*'"),
		DEF(AST_DIV,		"'/'"),
		DEF(AST_MOD,		"'%'"),
		DEF(AST_LIKE,		"'~'"),

		DEF(AST_LSHT,		"'<<'"),
		DEF(AST_RSHT,		"'>>'"),

		DEF(AST_BIT_OR,		"'|'"),
		DEF(AST_BIT_AND,	"'&'"),
		DEF(AST_BIT_XOR,	"'^'"),

		DEF(AST_LOG_OR,		"'or'"),
		DEF(AST_LOG_AND,	"'and'"),
		DEF(AST_LOG_XOR,	"'xor'"),
		DEF(AST_LOG_NOT,	"'not'"),


		/* reserved word */
		DEF(AST_IF,			"'if'"),
		DEF(AST_FUNC,		"'func'"),
		DEF(AST_SYSCALL,	"'syscall'"),
	#undef DEF
};


bool Parser::load(std::string src, std::string stmt) {
	std::fstream fs(src);
	std::string line;

	while (std::getline(fs, line)) {
		if ('\0' == line[0] || '#' == line[0]) {
			continue;
		}

		_D(LOG_DEBUG, "raw grammar - `%s`", line.c_str());

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

	return this->gentable(stmt, AST_ROOT);
}
bool Parser::load(std::string stmt, TOKENS rule, TOKENS front, TOKENS end) {
	TOKENS sub, tmp, ret;
	std::string line;

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
			case '[':	/* OR GROUP */
				for (size_t j = rule.size()-1; j > i; --j) {
					if ("]" == rule[j]) {
						/* update the front, cur, remainder parts */
						tmp = TOKENS(rule.begin(), rule.begin()+i);
						front.insert(front.end(), tmp.begin(), tmp.end());

						sub = TOKENS(rule.begin()+i+1, rule.begin()+j);

						tmp = TOKENS(rule.begin()+j+1, rule.end());
						end.insert(end.end(), tmp.begin(), tmp.end());

						return load(stmt, TOKENS{}, front, end) && load(stmt, sub, front, end);
					}
				}
				_D(LOG_WARNING, "parentheses are not paired");
				return false;
			default:
				break;
		}
	}

	/* concatenate into one rule */
	ret.insert(ret.end(), front.begin(), front.end());
	ret.insert(ret.end(), rule.begin(), rule.end());
	if (0 != end.size()) {
		/* NOTE - Handle the end part */
		return load(stmt, end, ret, TOKENS{});
	}


	for (auto it : ret) line += it + " ";
	_D(LOG_INFO, "set grammar - %-12s -> %s", stmt.c_str(), line.c_str());

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

	_D(LOG_DEBUG, "gentable on `%s` (W:%zu) with #%zu rules",
					_stmt.c_str(),
					_weight,
					this->_rules_[_stmt].size());
	for (auto rule : this->_rules_[_stmt]) {
		#ifdef DEBUG
			std::string line;

			for (auto it : rule) line += it + " ";
			_D(LOG_DEBUG, "grammar : %-8s -> %s", _stmt.c_str(), strip(line).c_str());
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
			} else {
				bool blFound = false;

				for (auto it : _relation_) {
					if (it.second.second == grammar) {
						for (auto _p : prev) {
							this->_table_[_p][it.first] = weight;
						}

						prev.clear();
						prev.push_back(it.first);
						blFound = true;
						break;
					}
				}

				if (false == blFound) {
					_D(LOG_CRIT, "Not define `%s`", grammar.c_str());
					goto END;
				}
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
	if (weight >= this->_stmt_.size()) {
		return "";
	} else {
		return this->_stmt_[weight].first;
	}
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
	int layout = 14;

	stream << "#ifndef __ZERG_PARSING_TABLE_H__" << std::endl;
	stream << "\t#define __ZERG_PARSING_TABLE_H__" << std::endl;

	{	/* original grammar rules */
		stream << "/* GRAMMAR rules\n * " << std::endl;
		for (const auto stmt : src._rules_) {
			for (auto rule : stmt.second) {
				std::string line = "";

				for (auto it : rule) line += it + " ";
				stream << " * "<< std::left << std::setw(layout) << stmt.first;
				stream << " -> " << line << "\n";
			}
			stream << " *\n";
		}
		stream << " */\n" << std::endl;
	}

#ifdef DEBUG
	{	/* parsing table with simplify grammar rules */
		stream << "/* Parsing Table\n *" << std::endl;
		for (auto it : _relation_) {
			if (it.first != AST_ROOT) {
				stream << std::left << std::setw(layout) << it.second.second;
			} else {
				stream << " * " << std::left << std::setw(layout) << " ";
			}
		}

		stream << std::endl;
		for (auto it : _relation_) {
			if (src._table_.end() == src._table_.find(it.first)) {
				continue;
			}

			stream << " * " << std::left << std::setw(12) << it.second.second;
			for (auto cur : _relation_) {
				std::map<_ASTType_, int> clm = src._table_.find(it.first)->second;

				if (cur.first == AST_ROOT) {
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
#endif /* DEBUG */

	{
		stream << "std::map<ASTType, std::map<ASTType, int>> _table_ = {\n\t";
		for (auto row : src._table_) {
			stream << "{\n\t\t" << _relation_[row.first].first << ", {\n";
			for (auto it : row.second) {
				stream << "\t\t\t{ " << _relation_[it.first].first << ",\t" <<it.second << "},\n";
			}
			stream << "\t\t},\n\t}, ";
		}
		stream << "\n};\n";
	}

	stream << "#endif /* __ZERG_PARSING_TABLE_H__ */";
	return stream;
}

