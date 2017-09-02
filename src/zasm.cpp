/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iostream>
#include <string>
#include <vector>

#include <unistd.h>
#include <getopt.h>

#include "zasm.h"

int  __verbose__ = 0;

void help(void) {
	fprintf(stderr, "ZASM (v%s)- Zerg assembler\n", ZASM_VERSION);
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage - zasm [option] src\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Option\n");
	fprintf(stderr, "    -f, --format           Output format -");
	#ifdef __x86_64__
		fprintf(stderr, " (x64)\n");
	#endif /* __x86_64__ */
	fprintf(stderr, "    -v, --verbose          Show the verbose message\n");
	fprintf(stderr, "    -o, --output <file>    Write output to <file>\n");
	fprintf(stderr, "        --pie              PIE - Position-Independent Executables\n");
	fprintf(stderr, "        --symbol           Show the symbol information\n");
	exit(-1);
}
int main(int argc, char *argv[]) {
	int optIdx = -1;
	char ch, opts[] = "f:vo:";
	std::string dst = "a.out";
	struct option options[] = {
		{"output",  optional_argument,	0, 'o'},
		{"pie",		no_argument,		0, 'p'},
		{"symbol",	optional_argument,	0, 'S'},
		{"verbose",	optional_argument,	0, 'v'},
		{"format",	optional_argument,  0, 'f'},
		{NULL, 0, 0, 0}
	};
	Args args = {
		.pie		= false,
		.symbol		= false,
		.entry		= 0x100000,
		.only_ir	= false,
		.compile_ir	= false,
		.no_stdlib	= false,
		#if defined(__APPLE__) && defined(__x86_64__)
		.fmt	= "macho64",
		#elif defined(__linux__) && defined(__x86_64__)
		.fmt	= "elf64",
		#endif /* __x86_64__ */
	};


	while (-1 != (ch = getopt_long(argc, argv, opts, options, &optIdx))) {
		switch(ch) {
			case 'f':
				args.fmt = optarg;
				break;
			case 'o':
				dst = optarg;
				break;
			case 'p':
				switch(optIdx) {
					case 1:		/* --pie */
						args.pie = true;
						break;
					default:
						_D(LOG_CRIT, "Not Implemented");
						break;
				}
				break;
			case 'S':
				switch(optIdx) {
					case 2:		/* --symbol */
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
		}
	}
	argc -= optind;
	argv += optind;

	if (0 == argc) {
		help();
		return -1;
	} else {
		Zasm *bin = new Zasm(dst, args);
		bin->assembleF(argv[0]);

		bin->dump();
		delete bin;
	}

	return 0;
}
/* vim set: tabstop=4 */

