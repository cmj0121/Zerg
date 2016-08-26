/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iostream>
#include <getopt.h>

#include "zerg.h"

int __verbose__ = 0;
void help(void) {
	fprintf(stderr, "ZERG - A useless compiler\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage - zerg [option]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Option\n");
	fprintf(stderr, "    -v, --verbose          Show the verbose message\n");
	exit(-1);
}
int main(int argc, char *argv[]) {
	int optIdx = -1;
	char ch, opts[] = "vo:";
	struct option options[] = {
		{"verbose", optional_argument, 0, 'v'},
		{NULL, 0, 0, 0}
	};

	while (-1 != (ch = getopt_long(argc, argv, opts, options, &optIdx))) {
		switch(ch) {
			case 'v':
				__verbose__++;
				break;
			default:
				_D(LOG_CRIT, "Not support option %c", ch);
				break;
		}
	}
	argc -= optind;
	argv += optind;

	if (0 == argc) {
		help();
	}
	return 0;
}
/* vim set: tabstop=4 */

