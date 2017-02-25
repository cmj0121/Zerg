/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include "zerg.h"

IRToken::IRToken(std::string line) {
	size_t cur = 0, pos = 0;

	for (cur = 0; cur <= line.size(); ++cur) {
		std::string token;

		switch(line[cur]) {
			case '\0': case '#':
				/* Need not process anymore */
				return;
			case ' ': case '\t':
				/* whitespace */
				break;
			default:
				for (pos = cur+1; pos <= line.size(); ++pos) {
					if (' ' == line[pos] || '\t' == line[pos]) break;
				}
				token = line.substr(cur, pos-cur);
				cur   = pos;
				break;
		}

		if (0 != token.length()) this->_src_.push_back(token);
	}
}
std::string IRToken::op(void) const {
	if ("->" == this->_src_[0]) {
		return "ASM";
	}
	return this->_src_[0];
}
std::string IRToken::dst(void) const {
	return 2 > this->length() ? "" : this->_src_[1];
}
std::string IRToken::src(void) const {
	return 3 > this->length() ? "" : this->_src_[2];
}
std::string IRToken::idx(void) const {
	return 4 > this->length() ? "" : this->_src_[3];
}
std::string IRToken::size(void) const {
	return 5 > this->length() ? "" : this->_src_[4];
}

size_t IRToken::length(void) const {
	/* return the length of the IR */
	return this->_src_.size();
}
std::ostream& operator << (std::ostream &stream, const IRToken &src) {
	std::string dst;

	for (auto it : src._src_) dst += it + " ";
	stream << dst;
	return stream;
}
