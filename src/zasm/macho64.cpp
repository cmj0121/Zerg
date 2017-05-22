/* Copyright (C) 2014-2016 cmj. All right reserved. */

#ifdef __APPLE__

#include <sys/stat.h>
#include "zasm.h"

#include "macho64_inc.hpp"


void Binary::dump(off_t entry, bool showSymb) {
	off_t header_offset = 0;
	std::fstream fp;
	std::vector<std::string> symbs;
	std::vector<std::pair<std::string, struct nlist_64>> symblist;

	Binary::reallocreg();
	fp.open(_dst_, std::fstream::out | std::fstream::binary | std::fstream::trunc);

	/* Create necessary header, dummy first */
	for (int i = 0; i < 2; ++i) {
		off_t symboff = 1, binoff = 0;

		header(fp, 8, header_offset, this->_pie_);

		seg_pagezero(fp);
		seg_text(fp, Binary::length(), entry, header_offset);
		seg_linkedit(fp, showSymb ? symbs : std::vector<std::string>{});
		dyld_info(fp);
		seg_symtab(fp, showSymb ? symbs : std::vector<std::string>{});
		seg_dysymtab(fp);
		dyld_link(fp);
		seg_unixthread(fp, entry, header_offset);

		header_offset 	= fp.tellg();
		binoff			= entry + header_offset;

		if (0 == i) {
			for (unsigned int idx = 0; idx < _inst_.size(); ++idx) {
				std::string symb = _inst_[idx]->label();

				if (_inst_[idx]->isShowLabel()) {
					struct nlist_64 symlist;

					symlist.n_un.n_strx = symboff;
					symlist.n_type		= N_TYPE;
					symlist.n_sect		= 0x01;
					symlist.n_desc		= REFERENCE_FLAG_UNDEFINED_NON_LAZY;
					symlist.n_value		= binoff;

					if (symbs.end() != std::find(symbs.begin(), symbs.end(), symb)) {
						/* NOTE - duplicated symbol detected */
						_D(LOG_CRIT, "Duplicate symbol `%s`", symb.c_str());
					}

					symblist.push_back(std::make_pair(symb,  symlist));
					symbs.push_back(symb);
					symboff += symb.size() + 1;
				}

				/* offset of the symbol address */
				binoff += _inst_[idx]->length();
			}
		}
	}

	/* Write machine code */
	for (int idx = 0; idx < _inst_.size(); ++idx) {
		(*_inst_[idx]) << fp;
	}

	/* HACK - Mach-O 64 always need large than 4K */
	while (0x1000 > fp.tellg()) {
		fp.write("\x00", 1);
	}

	/* dump the symbol */
	if (showSymb) {
		for (auto it : symblist) {
			fp.write((char *)&it.second, sizeof(struct nlist_64));
		}
		fp.write("\x00", 1);
		for (auto it : symblist) {
			fp.write(it.first.c_str(), it.first.size());
			fp.write("\x00", 1);
		}
	}

	fp.close();
	chmod(_dst_.c_str(), 0755);
}

#endif /* __APPLE__ */
/* vim set: tabstop=4 */

