/* Copyright (C) 2014-2017 cmj. All right reserved. */
#if __linux__

#include <vector>

#include <string.h>
#include <elf.h>
#include <sys/stat.h>
#include "zasm.h"

void Zasm::dump_elf64(off_t entry, bool symb) {
	Elf64_Ehdr header = {0};
	Elf64_Phdr segment = {0};
	std::fstream fp;

	Zasm::reallocreg();
	fp.open(_dst_, std::fstream::out | std::fstream::binary | std::fstream::trunc);

	{
		segment.p_type		= PT_LOAD;
		segment.p_flags		= PF_R | PF_X;
		segment.p_offset	= 0x0;
		segment.p_vaddr		= entry;
		segment.p_paddr		= entry;
		segment.p_filesz	= 0x0;
		segment.p_memsz		= 0x0;
		segment.p_align		= 0x1000;
	}

	{
		header.e_ident[EI_MAG0]		= ELFMAG0;
		header.e_ident[EI_MAG1]		= ELFMAG1;
		header.e_ident[EI_MAG2]		= ELFMAG2;
		header.e_ident[EI_MAG3]		= ELFMAG3;
		header.e_ident[EI_CLASS]	= ELFCLASS64;
		header.e_ident[EI_DATA]		= ELFDATA2LSB;
		header.e_ident[EI_VERSION]	= EV_CURRENT;
		header.e_ident[EI_OSABI]	= ELFOSABI_SYSV;


		header.e_type		= ET_EXEC;
		header.e_machine	= EM_X86_64;
		header.e_version	= EV_CURRENT;
		header.e_entry		= entry + sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr);
		header.e_phoff		= sizeof(Elf64_Ehdr);
		header.e_shoff		= 0x0;
		header.e_flags		= 0x0;
		header.e_ehsize		= sizeof(header);
		header.e_phentsize	= sizeof(Elf64_Phdr) * 0x01;
		header.e_phnum		= 0x01;
		header.e_shentsize	= sizeof(Elf64_Section) * 0x0;
		header.e_shnum		= 0x0;
		header.e_shstrndx	= 0x0;
	}


	fp.write((char *)&header, sizeof(header));
	fp.write((char *)&segment, sizeof(Elf64_Phdr));

	/* Write machine code */
	for (unsigned int idx = 0; idx < _inst_.size(); ++idx) {
		(*_inst_[idx]) << fp;
	}

	segment.p_filesz	= fp.tellg();
	segment.p_memsz		= segment.p_filesz;

	fp.seekp(sizeof(header));
	fp.write((char *)&segment, sizeof(Elf64_Phdr));

	fp.close();
	chmod(_dst_.c_str(), 0755);
}

#endif /* __linux__ */
/* vim set: tabstop=4 */

