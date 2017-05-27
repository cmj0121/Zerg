/* Copyright (C) 2014-2017 cmj <cmj@cmj.tw>. All right reserved. */

#include <vector>
#include "zerg.h"

int __verbose__;

class ZergLexerTest : public Zerg {
	public:
		ZergLexerTest(std::string dst, Args &args) : Zerg(dst, args) {};

		void compile(std::string srcfile, std::vector<std::string> checklist);
		std::string parser(std::string token, std::string prev);
	private:
		int _nr_;
		std::vector<std::string> _checklist_;
};

void ZergLexerTest::compile(std::string srcfile, std::vector<std::string> checklist) {
	this->_checklist_ = checklist;
	this->_nr_        = 1;
	Zerg::compile(srcfile);

	if (0 != _checklist_.size()) {
		_D(LOG_CRIT, "Too many token expect - %s (#%d)",	_checklist_[0].c_str(),
															this->_nr_);
	}
}
std::string ZergLexerTest::parser(std::string token, std::string prev) {
	if (0 == _checklist_.size()) {
		_D(LOG_CRIT, "Missing token - %s", token.c_str());
	} else if (_checklist_[0] != token) {
		_D(LOG_CRIT, "Token not match - %s / %s #%d",
						token.c_str(), _checklist_[0].c_str(), this->_nr_);
	}

	_checklist_.erase(_checklist_.begin());
	_nr_ ++;
	return token;
}


int main() {
	Args args {
		.no_stdlib		= true,
	};
	ZergLexerTest agent("a.out", args);

	agent.compile("testcases/lexer_1.zg", {"print", "\"abc\"", "\n"});
	agent.compile("testcases/lexer_2.zg", {"1", "**", "2", "-", "~", "3", "%", "4", "\n"});
	agent.compile("testcases/lexer_3.zg", {	"if", "Foo","(", "x", ")",	":", "\n",
												LEXER_INDENT, "print", "'true case'", "\n",
											LEXER_DEDENT, "\n"});
}
