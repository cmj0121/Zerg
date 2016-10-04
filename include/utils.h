/* Copyright (C) 2014-2016 cmj. All right reserved. */
#ifndef _ZERG_UTILS_H_
#  define _ZERG_UTILS_H_

extern int __verbose__;
enum ZergLogLevel {
	LOG_CRIT	= 0,
	LOG_BUG,
	LOG_WARNING,
	LOG_DEBUG,
	LOG_INFO,
};

#include <string>
#include <vector>
typedef std::string STRING;

#include <stdio.h>
#define _D(lv, msg, ...) 										\
	do {														\
		if (lv <= __verbose__) {								\
			fprintf(stderr, "%-16s L#%04d (%02d) - " msg "\n",	\
					__FILE__, __LINE__, lv, ##__VA_ARGS__);		\
		}														\
	} while (0)
#define _DEBUG()	_D(0, "\x1b[1;34m ~ debug ~\x1b[m")

#define StrCP(dst, src)		snprintf(dst, sizeof(dst), "%s", src)
#define ARRAY_SIZE(ctx)		(sizeof(ctx)/sizeof(ctx[0]))
#define FREE(ctx)			{ if (NULL != ctx) free(ctx); (ctx) = NULL; }

inline static std::string strip(std::string src) {
	if (' ' != src[0] && ' ' != src[src.size()-1]) {
		return src;
	}

	size_t _start, _end;
	_start = src.find_first_not_of(' ');
	_end   = src.find_last_not_of(' ');

	_start = _start == std::string::npos ? 0 : _start;
	_end   = _end == std::string::npos ? src.size()-1 : _end;
	return src.substr(_start, _end-_start+1);
}
inline static std::vector<std::string> split(std::string src, std::string tok) {
	size_t pos = 0;
	std::string remainder = src;
	std::vector<std::string> ret;

	while (std::string::npos != (pos = remainder.find_first_of(tok))) {
		std::string tmp;

		tmp = remainder.substr(0, pos);
		if ('\0' != tmp[0]) ret.push_back(tmp);
		remainder = remainder.substr(pos+tok.size());
	}

	if ('\0' != remainder[0]) ret.push_back(remainder);

	return ret;
}
#endif /* _ZERG_UTILS_H_ */

