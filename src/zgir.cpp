/* Copyright (C) 2014-2017 cmj. All right reserved. */

#include <iostream>
#include <getopt.h>

#include "zerg.h"

int  __verbose__  = 0;
void help(void) {
	fprintf(stderr, "ZERG IR (v%s)- A useless compiler from Zerg IR\n", __IR_VERSION__);
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage - zgir [option]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Option\n");
	fprintf(stderr, "    -o, --output           Output file\n");
	fprintf(stderr, "        --pie              PIE - Position-Independent Executables\n");
	fprintf(stderr, "        --symbol           Show the symbol information\n");
	exit(-1);
}

int main(int argc, char *argv[]) {
	int optIdx = -1;
	char ch, opts[] = "hpSv";
	struct option options[] = {
		{"help",		no_argument,		0, 'h'},
		{"output",		required_argument,	0, 'o'},
		{"pie",			no_argument,		0, 'p'},
		{"symbol",		optional_argument,	0, 'S'},
		{"verbose",		optional_argument,	0, 'v'},
		{NULL, 0, 0, 0}
	};
	std::string dst = "a.out";
	Args args = {
		.entry			= 0x1000,
		.only_ir		= false,
		.pie			= false,
		.symbol			= false,
		.compile_ir		= false,
		.no_stdlib		= false,
	};


	while (-1 != (ch = getopt_long(argc, argv, opts, options, &optIdx))) {
		switch(ch) {
			case 'h':
				help();
				return -1;
			case 'o':
				dst = optarg;
				break;
			case 'p':
				switch(optIdx) {
					case 2:		/* --pie */
						args.pie = true;
						break;
					default:
						_D(LOG_CRIT, "Not Implemented");
						break;
				}
				break;
			case 'S':
				switch(optIdx) {
					case 3:		/* --symbol */
						args.symbol = true;
						break;
					default:
						_D(LOG_CRIT, "Not Implemented");
						break;
				}
				break;
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
	} else {
		IR *ir = new IR(dst, args);

		ir->compile(argv[0]);
		delete ir;
	}

	return 0;
}
/* vim set: tabstop=4 */
