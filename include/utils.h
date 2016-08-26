/* Copyright (C) 2014-2016 cmj. All right reserved. */
#ifndef _ZERG_UTILS_H_
#  define _ZERG_UTILS_H_

#include <iostream>

enum ZergLogLevel {
	LOG_CRIT	= 0,
	LOG_BUG,
	LOG_WARNING,
	LOG_DEBUG,
	LOG_INFO,
};

#define _D(lv, msg, ...) 										\
	do {														\
		if (lv <= __verbose__) {								\
			fprintf(stderr, "%-16s L#%04d (%02d) - " msg "\n",	\
				 __FILE__, __LINE__, lv, ##__VA_ARGS__);		\
		}														\
	} while (0)
#define _DEBUG()	_D(0, "\x1b[1;34m ~ debug ~\x1b[m")

#define _E(msg, ...) {		\
		fprintf(stderr, "%-16s L#%04d - " msg "\n",	\
			__FILE__, __LINE__, ##__VA_ARGS__);	\
		exit(-1);									\
	}
#define _S(stmt)														\
		do {															\
			_E("Syntax Error - '%s'", this->dump_expr(stmt).c_str());	\
		} while (0)

#define NOT_IMPLEMENTED	_E("Not Implemented");

#define StrCP(dst, src) snprintf(dst, sizeof(dst), "%s", src)
#define ARRAY_SIZE(ctx) (sizeof(ctx)/sizeof(ctx[0]))

#define FREE(ctx) { if (NULL != ctx) free(ctx); (ctx) = NULL; }

#endif /* _ZERG_UTILS_H_ */

