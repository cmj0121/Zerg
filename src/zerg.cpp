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
	fprintf(stderr, "ZERG - A useless compiler\n");
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
	char ch, opts[] = "Go:nprRvh";
	struct option options[] = {
		{"grammar",		no_argument,		0, 'G'},
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
	ZergArgs args = {
		._entry_		= 0x1000,
		._only_ir_		= false,
		._gen_grammar	= false,
		._pie_			= false,
		._symbol_		= false,
		._compile_ir_	= false,
		._no_stdlib_	= false
	};


	while (-1 != (ch = getopt_long(argc, argv, opts, options, &optIdx))) {
		switch(ch) {
			case 'G':
				args._gen_grammar = true;
				break;
			case 'h':
				help();
				return -1;
			case 'o':
				dst = optarg;
				break;
			case 'n':
				switch(optIdx) {
					case 7:		/* --no-stdlib */
						args._no_stdlib_ = true;
						break;
					default:
						_D(LOG_CRIT, "Not Implemented");
						break;
				}
				break;
			case 'p':
				switch(optIdx) {
					case 3:		/* --pie */
						args._pie_ = true;
						break;
					default:
						_D(LOG_CRIT, "Not Implemented");
						break;
				}
				break;
			case 'r':
				args._only_ir_ = true;
				break;
			case 'R':
				args._compile_ir_ = true;
				break;
			case 'S':
				switch(optIdx) {
					case 5:		/* --symbol */
						args._symbol_ = true;
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
		Zerg zerg(dst, &args);
		zerg.compile(argv[0], &args);
	}

	return 0;
}
/* vim set: tabstop=4 */

