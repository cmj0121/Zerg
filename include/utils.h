/* Copyright (C) 2014-2016 cmj. All right reserved. */
#ifndef _ZERG_UTILS_H_
#  define _ZERG_UTILS_H_

#if __APPLE__
	#define OFF_T	"%llX"
#elif __linux__
	#define OFF_T	"%zX"
#endif /* end-of OS detected */

/* general help function */
#include <stdio.h>
#define StrCP(dst, src)		snprintf((char *)dst, sizeof(dst), "%s", src)
#define ARRAY_SIZE(ctx)		(sizeof(ctx)/sizeof(ctx[0]))
#define FREE(ctx)			{ if (NULL != ctx) free(ctx); (ctx) = NULL; }

#include <string>
typedef std::string STRING;

#include <string>
#include <vector>
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

/* Log-Related */
extern int __verbose__;

enum ZergLogLevel {
	LOG_CRIT	= -1,	/* critical issue, need stop program */
	LOG_ERROR,			/* system error */
	LOG_WARNING,		/* warning message */
	LOG_INFO,			/* information */
	LOG_DEBUG,			/* debug message */
	LOG_DEBUG2,
	LOG_DEBUG3,

	ZASM_LOG_WARNING,
	ZASM_LOG_INFO,
	ZASM_LOG_DEBUG,

	LOG_DEBUG_LEXER,
	LOG_REGISTER_ALLOC,
	ZASM_LOG_DISASM		= 1,
};

#include <stdio.h>

#define _D(lv, msg, ...) 										\
	do {														\
		if (LOG_CRIT == lv) {									\
			fprintf(stderr, "%-16s L#%04d - " msg "\n",			\
					__FILE__, __LINE__, ##__VA_ARGS__);			\
			exit(-1);											\
		} else if (lv <= __verbose__) {							\
			fprintf(stderr, "%-16s L#%04d (%02d) - " msg "\n",	\
					__FILE__, __LINE__, lv, ##__VA_ARGS__);		\
		}														\
	} while (0)
#define _S(tokens)									\
	do {											\
		std::string line;							\
		for (auto &it : tokens) line += it + " ";	\
		_D(LOG_CRIT, "Syntax Error `%s` on %s", 	\
				token.c_str(), line.c_str());		\
	} while (0)
#define _DEBUG()	_D(0, "\x1b[1;34m ~ debug ~\x1b[m")
#define ALERT(expr)								\
	if (expr) {									\
		_D(LOG_CRIT, "BUG - `%s`", #expr);	\
	}

#endif /* _ZERG_UTILS_H_ */

