/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iostream>
#include <getopt.h>

#include "zerg.h"

int  __verbose__  = 0;
int  _entry_      = 0x1000;
bool _only_ir_    = false;
bool _gen_grammar = false;
bool _pie_        = false;
bool _symbol_     = false;
bool _compile_ir_ = false;
bool _no_stdlib_  = false;

void help(void) {
	fprintf(stderr, "ZERG (v%s)- A useless compiler\n", ZERG_VERSION);
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage - zerg [option]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Option\n");
	fprintf(stderr, "    -G, --grammar          Generate the new grammar header\n");
	fprintf(stderr, "    -o, --output           Output file\n");
	fprintf(stderr, "    -r, --ir               Show the IR\n");
	fprintf(stderr, "    -R, --compile-ir       Compile from the Zerg IR\n");
	fprintf(stderr, "    -v, --verbose          Show the verbose message\n");
	fprintf(stderr, "        --no-stdlib        No stander library\n");
	fprintf(stderr, "        --pie              PIE - Position-Independent Executables\n");
	fprintf(stderr, "        --symbol           Show the symbol information\n");
	exit(-1);
}
int main(int argc, char *argv[]) {
	int optIdx = -1;
	char ch, opts[] = "o:nprRvh";
	struct option options[] = {
		{"help",		no_argument,		0, 'h'},
		{"output",		required_argument,	0, 'o'},
		{"pie",			no_argument,		0, 'p'},
		{"ir",			no_argument, 		0, 'r'},
		{"symbol",		optional_argument,	0, 'S'},
		{"verbose",		optional_argument,	0, 'v'},
		{"no-stdlib",	no_argument,		0, 'n'},
		{NULL, 0, 0, 0}
	};
	std::string dst = "a.out";
	Args args = {
		.pie			= false,
		.symbol			= false,
		.entry			= 0x1000,
		.only_ir		= false,
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
			case 'n':
				switch(optIdx) {
					case 6:		/* --no-stdlib */
						args.no_stdlib = true;
						break;
					default:
						_D(LOG_CRIT, "Not Implemented");
						break;
				}
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
			case 'r':
				args.only_ir = true;
				break;
			case 'R':
				args.compile_ir = true;
				break;
			case 'S':
				switch(optIdx) {
					case 4:		/* --symbol */
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
	} else if (_gen_grammar) {
		_D(LOG_CRIT, "Not Implemented");
	} else {
		Zerg zerg(dst, args);
		zerg.compile(argv[0]);
	}

	return 0;
}
/* vim set: tabstop=4 */

