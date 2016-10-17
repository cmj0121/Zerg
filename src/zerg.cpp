/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iostream>
#include <getopt.h>

#include "zerg.h"

int  __verbose__  = 0;
bool _only_ir_	  = false;
bool _gen_grammar = false;

void help(void) {
	fprintf(stderr, "ZERG - A useless compiler\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage - zerg [option]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Option\n");
	fprintf(stderr, "    -G, --grammar          Generate the new grammar header\n");
	fprintf(stderr, "    -o, --output           Output file\n");
	fprintf(stderr, "    -r, --ir               Show the IR\n");
	fprintf(stderr, "    -v, --verbose          Show the verbose message\n");
	exit(-1);
}
int main(int argc, char *argv[]) {
	int optIdx = -1;
	char ch, opts[] = "Go:rvh";
	struct option options[] = {
		{"grammar",	no_argument,		0, 'G'},
		{"help",	no_argument,		0, 'h'},
		{"output",	required_argument,	0, 'o'},
		{"ir",		no_argument, 		0, 'r'},
		{"verbose", optional_argument,	0, 'v'},
		{NULL, 0, 0, 0}
	};
	std::string dst = "a.out";

	while (-1 != (ch = getopt_long(argc, argv, opts, options, &optIdx))) {
		switch(ch) {
			case 'G':
				_gen_grammar = true;
				break;
			case 'h':
				help();
				return -1;
			case 'o':
				dst = optarg;
				break;
			case 'r':
				_only_ir_ = true;
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
	} else if (_gen_grammar) {
		ParsingTable parser;

		parser.load(argv[0]);
		std::cout << parser << std::endl;
	} else {
		Zerg zerg(dst);
		zerg.compile(argv[0], _only_ir_);
	}

	return 0;
}
/* vim set: tabstop=4 */

