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
	fprintf(stderr, "        --pie              PIE - Position-Independent Executables\n");
	fprintf(stderr, "        --symbol           Show the symbol information\n");
	exit(-1);
}
int main(int argc, char *argv[]) {
	int optIdx = -1;
	char ch, opts[] = "Go:prRvh";
	struct option options[] = {
		{"grammar",		no_argument,		0, 'G'},
		{"help",		no_argument,		0, 'h'},
		{"output",		required_argument,	0, 'o'},
		{"pie",			no_argument,		0, 'p'},
		{"ir",			no_argument, 		0, 'r'},
		{"symbol",		optional_argument,	0, 'S'},
		{"verbose",		optional_argument,	0, 'v'},
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
			case 'p':
				switch(optIdx) {
					case 3:		/* --pie */
						_pie_ = true;
						break;
					default:
						_D(LOG_CRIT, "Not Implemented");
						break;
				}
				break;
			case 'r':
				_only_ir_ = true;
				break;
			case 'R':
				_compile_ir_ = true;
				break;
			case 'S':
				switch(optIdx) {
					case 5:		/* --symbol */
						_symbol_ = true;
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
		Zerg zerg(dst, _pie_, _entry_, _symbol_);
		zerg.compile(argv[0], _only_ir_, _compile_ir_);
	}

	return 0;
}
/* vim set: tabstop=4 */

