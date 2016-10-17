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

ParsingTable::ParsingTable(void) : _stmt_(NULL) {
	this->_stmt_ = new STR_TREE("stmt");
}

bool ParsingTable::load(std::string src, std::string stmt) {
	std::fstream fs(src);
	std::string line;

	/* re-build the parsing table */
	this->_table_.clear();

	while (std::getline(fs, line)) {
		if ('\0' == line[0] || '#' == line[0]) {
			continue;
		}

		/* process the grammar rule `stmt : rules` */
		size_t pos = 0;
		std::string stmt;
		TOKENS rule, dummy1, dummy2;

		if (std::string::npos == (pos = line.find(':'))) {
			_D(LOG_CRIT, "Not Valid - `%s`", line.c_str());
			exit(-1);
		}

		stmt = strip(line.substr(0, pos));
		rule = split(strip(line.substr(pos+1)), " ");

		if (false == load(stmt, rule, dummy1, dummy2)) {
			/* grammar rule is not valid */
			_D(LOG_CRIT, "Invalid grammar rule - `%s`", line.c_str());
		}
	}

	this->gentable(stmt, AST_ROOT);
	return true;
}
bool ParsingTable::load(std::string stmt, TOKENS rule, TOKENS front, TOKENS end) {
	TOKENS dummy;

	/* remove all GROUP and OR */
	for (size_t i = 0; i < rule.size(); ++i) {
		switch(rule[i][0]) {
			case '|':
				do {
					TOKENS	sub1(rule.begin(), rule.begin()+i),
							sub2(rule.begin()+i+1, rule.end());

					return load(stmt, sub1, front, end) && load(stmt, sub2, front, end);
				} while (0);
			case '(':
				for (size_t j = rule.size()-1; j > i; --j) {
					if (')' == rule[j][0]) {
						TOKENS	tmp = TOKENS(rule.begin(), rule.begin()+i),
								sub = TOKENS(rule.begin()+i+1, rule.begin()+j);

						/* update the front, cur, remainder parts */
						front.insert(front.end(), tmp.begin(), tmp.end());

						tmp = TOKENS(rule.begin()+j+1, rule.end());
						tmp.insert(tmp.end(), end.begin(), end.end());

						bool blRet = false;
						switch(rule[j][rule[j].size()-1]) {
							case ')':
								blRet = load(stmt, sub, front, tmp);
								break;
							case '*':
								ALERT(rule[j-1] != (i == 0 ? front[front.size()-1] : rule[j-1]));

								blRet = load(stmt, dummy, front, tmp) && load(stmt, sub, front, tmp);
								break;
							default:
								blRet = false;
								break;
						}

						return blRet;
					}
				}
				_D(LOG_WARNING, "parentheses are not paired");
				return false;
			case '[':
				for (size_t j = rule.size()-1; j > i; --j) {
					if ("]" == rule[j] && "[" == rule[i]) {
						TOKENS	tmp = TOKENS(rule.begin(), rule.begin()+i),
								sub = TOKENS(rule.begin()+i+1, rule.begin()+j),
								dummy;

						/* update the front, cur, remainder parts */
						front.insert(front.end(), tmp.begin(), tmp.end());

						tmp = TOKENS(rule.begin()+j+1, rule.end());
						tmp.insert(tmp.end(), end.begin(), end.end());
						return load(stmt, dummy, front, tmp) && load(stmt, sub, front, tmp);
					}
				}
				_D(LOG_WARNING, "parentheses are not paired");
				return false;
			default:
				break;
		}
	}

	if (0 != end.size()) {
		/* NOTE - Handle the end part */
		TOKENS dummy;

		front.insert(front.end(), rule.begin(), rule.end());
		return load(stmt, end, front, dummy);
	}
	front.insert(front.end(), rule.begin(), rule.end());
	front.insert(front.end(), end.begin(), end.end());

	std::string line;
	for (auto it : front) line += it + " ";
	_D(LOG_DEBUG, "set grammar - %12s -> %s", stmt.c_str(), line.c_str());

	/* save into rule map */
	if (this->_rules_.end() == this->_rules_.find(stmt)) {
		this->_rules_[stmt] = { front };
	} else {
		this->_rules_[stmt].push_back(front);
	}

	return true;
}
void ParsingTable::gentable(std::string stmt, ASTType _prev) {
	int weight = -1, cnt = 0;
	std::vector<ASTType> cached;
	STR_TREE *curstmt = this->_stmt_->find(stmt);

	for (auto it : this->_processed_) {
		if (it.first == stmt && it.second == _prev) {
			return ;
		}
	}

	ALERT(NULL == curstmt || -1 == (weight = curstmt->height()));
	for (auto rule : this->_rules_[stmt]) {
		std::string line;
		std::vector<ASTType> cur = { _prev };

		for (auto it : rule) line += it + " ";
		_D(LOG_INFO, "#%2d/%2zu (F:0x%02X, W:%02d) %12s -> %s",
						++cnt, this->_rules_[stmt].size(),
						_prev, weight,
						stmt.c_str(), line.c_str());

		for (auto token : rule) {
			_D(LOG_DEBUG, "process %s", token.c_str());
			ALERT(token == stmt);

			if ('+' == token[token.size()-1]) {			/* MORE */
				_D(LOG_CRIT, "Not Implemented");
				gentable(token.substr(0, token.size()-1), _prev);
				continue;
			} else if ('a' <= token[0] && 'z' >= token[0]) {	/* NEXT */
				curstmt->insert(token);
				for (auto _p : cur) {
					gentable(token, _p);
				}
				cur = this->_cache_[token];
				continue;
			} else {											/* SYNTAX */
				bool blFound = false;

				for (auto it : _relation_) {
					if (it.second.second != token) continue;

					for (auto _p : cur) {
						this->_table_[_p][it.first] = weight;
					}

					cur.clear();
					cur.push_back(it.first);
					blFound = true;
					break;
				}

				if (false == blFound) {
					/* special token not defined */
					_D(LOG_CRIT, "grammar `%s` not define", token.c_str());
				}
			}
		}

		for (auto it : cur) {
			if (cached.end() == std::find(cached.begin(), cached.end(), it)) {
				cached.push_back(it);
			}
		}
	}

	this->_cache_[stmt] = cached;
	this->_processed_.push_back(std::make_pair(stmt, _prev));
}

int ParsingTable::weight(ASTType prev, ASTType cur) {
	int weight = -1;

	_D(LOG_DEBUG, "get weight %d %d on parsing table", prev, cur);

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

std::ostream& operator <<(std::ostream &stream, const ParsingTable &src) {
	int layout = 14;

	{	/* original grammar rules */
		stream << "/* GRAMMAR rules\n * " << std::endl;
		for (auto stmt : src._rules_) {
			for (auto rule : stmt.second) {
				std::string line = "";

				for (auto it : rule) line += it + " ";
				stream << " * " << std::setw(layout) << stmt.first;
				stream << " -> " << line << "\n";
			}
			stream << " *\n";
		}
		stream << " */\n" << std::endl;
	}

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
	return stream;
}

