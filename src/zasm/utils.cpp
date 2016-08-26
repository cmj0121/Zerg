/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iomanip>
#include "zasm.h"

Utils::~Utils() {
}

bool Utils::isInt(std::string src) {
	bool blRet = false;

	if ("0x" == src.substr(0, 2) || "0X" == src.substr(0, 2)) {
		for (int idx = 2; idx < src.size(); ++idx) {
			if ('0' <= src[idx] && src[idx] <= '9') {
				continue;
			} else if ('a' <= (src[idx] | 0x20) && (src[idx] | 0x20) <= 'f') {
				continue;
			}
			goto END;
		}
	} else {
		for (int idx = 0; idx < src.size(); ++idx) {
			if ('0' <= src[idx] && src[idx] <= '9') {
				continue;
			}
			goto END;
		}
	}
	blRet = true;
END:
	return blRet;
}
off_t Utils::toInt(std::string src) {
	if ('&' == src[0]) {
		return 0xFFFFFFFF;
	} else {
		std::stringstream ss;
		off_t _src_;

		if ("0x" == src.substr(0, 2) || "0X" == src.substr(0, 2)) {
			ss << std::hex << src;
		} else {
			ss << src;
		}
		ss >> _src_;
		return _src_;
	}
}
std::string Utils::toHex(unsigned char *payload, size_t size) {
	std::stringstream ss;

	for(size_t idx = 0; idx < size; ++idx) {
		ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2)
			<< (int)payload[idx] << " ";
	}

	return ss.str();
}
std::string Utils::unescape(std::string src) {
	std::string dst = "";

	for (int i = 0; i < src.size(); ++i) {
		if ('\\' != src[i]) {
			dst += src[i];
		} else {
			switch (src[++i]) {
				case 'a':
					dst += '\a';
					break;
				case 'n':
					dst += '\n';
					break;
				default:
					dst += '\\' + src[i];
					break;
			}
		}
	}
	return dst;
};
