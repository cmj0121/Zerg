/* Copyright (C) 2014-2016 cmj. All right reserved. */
#ifndef _ZERG_UTILS_H_
#  define _ZERG_UTILS_H_

enum ZergLogLevel {
	LOG_CRIT	= 0,
	LOG_BUG,
	LOG_WARNING,
	LOG_DEBUG,
	LOG_INFO,
};

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

#endif /* _ZERG_UTILS_H_ */

