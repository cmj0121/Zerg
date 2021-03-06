
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/stab.h>
#include <mach/vm_prot.h>

static void header(std::fstream &src, int ncmds, off_t offset, bool pie) {
	struct mach_header_64 hdr;

	src.seekp(0);
	memset(&hdr, 0x00, sizeof(hdr));
	hdr.magic			= MH_MAGIC_64;
	hdr.cputype			= CPU_TYPE_X86_64;
	hdr.cpusubtype		= CPU_SUBTYPE_LIB64 | CPU_SUBTYPE_VAX750;
	hdr.filetype		= MH_EXECUTE;
	hdr.ncmds			= ncmds;
	hdr.sizeofcmds		= offset - sizeof(struct mach_header_64);
	hdr.flags			= MH_NOUNDEFS | MH_DYLDLINK | MH_TWOLEVEL | (pie ? MH_PIE : 0);
	hdr.reserved		= 0x0;

	src.write((char *)&hdr, sizeof(struct mach_header_64));
}
static void seg_pagezero(std::fstream &src) {
	struct segment_command_64		hdr;

	memset(&hdr, 0x00, sizeof(hdr));
	hdr.cmd			= LC_SEGMENT_64;
	hdr.cmdsize		= sizeof(struct segment_command_64);
	StrCP(hdr.segname, SEG_PAGEZERO);
	hdr.vmaddr		= 0x0;
	hdr.vmsize		= 0x1000;
	hdr.fileoff		= 0x0;
	hdr.filesize	= 0x0;
	hdr.maxprot		= 0x0;
	hdr.initprot	= 0x0;
	hdr.nsects		= 0x0;
	hdr.flags		= 0x0;

	src.write((char *)&hdr, sizeof(struct segment_command_64));
}

static void sec_text(std::fstream &src, int size, off_t entry, off_t offset) {
	struct section_64 hdr;

	memset(&hdr, 0x00, sizeof(hdr));
	StrCP(hdr.sectname, SECT_TEXT);
	StrCP(hdr.segname, SEG_TEXT);
	hdr.addr		= entry + offset;
	hdr.size		= size;
	hdr.offset		= offset;
	hdr.align		= 0x4;
	hdr.reloff		= 0x0;
	hdr.nreloc		= 0x0;
	hdr.flags		= S_ATTR_PURE_INSTRUCTIONS | S_ATTR_SOME_INSTRUCTIONS;
	hdr.reserved1	= 0x0;
	hdr.reserved2	= 0x0;
	hdr.reserved3	= 0x0;

	src.write((char *)&hdr, sizeof(struct section_64));
}

static void seg_text(std::fstream &src, int size, off_t entry, off_t offset) {
	struct segment_command_64 hdr;

	memset(&hdr, 0x00, sizeof(hdr));
	hdr.cmd		= LC_SEGMENT_64;
	hdr.cmdsize	= sizeof(struct segment_command_64) + sizeof(struct section_64);
	StrCP(hdr.segname, SEG_TEXT);
	hdr.vmaddr		= entry;
	hdr.vmsize		= 0x1000000;	/* 1M - NOTE when total size > 1M need enlarge this value */
	hdr.fileoff		= 0x0;
	hdr.filesize	= size + offset;
	hdr.maxprot		= VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE;
	hdr.initprot	= VM_PROT_READ | VM_PROT_EXECUTE;
	hdr.nsects		= 0x1;
	hdr.flags		= 0x0;

	src.write((char *)&hdr, sizeof(segment_command_64));
	sec_text(src, size, entry, offset);
}

static void seg_linkedit (std::fstream &src, std::vector<std::string> symb) {
	struct segment_command_64 hdr;
	uint32_t strlen = 0;

	for (auto it : symb) {
		strlen += it.size() + 1;
	}

	strlen += symb.size() * sizeof(struct nlist_64);

	hdr.cmd		= LC_SEGMENT_64;
	hdr.cmdsize	= sizeof(struct segment_command_64);
	StrCP(hdr.segname, SEG_LINKEDIT);
	hdr.vmaddr		= 0x0;
	hdr.vmsize		= 0x0;
	hdr.fileoff		= 0x1000;	/* Don't know why need large than 4K */
	hdr.filesize	= strlen;
	hdr.maxprot		= VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE;
	hdr.initprot	= VM_PROT_READ;
	hdr.nsects		= 0x0;
	hdr.flags		= 0x0;

	src.write((char *)&hdr, sizeof(struct segment_command_64));
}

static void seg_unixthread(std::fstream &src, int entry, off_t offset) {
	uint32_t				flavor, count;
	struct thread_command	thread;
	x86_thread_state64_t	thread_state;

	thread.cmd		= LC_UNIXTHREAD;
	thread.cmdsize	= sizeof(struct thread_command);
	thread.cmdsize += sizeof(x86_thread_state64_t) + 2*sizeof(uint32_t);
	flavor	= x86_THREAD_STATE64;
	count	= x86_THREAD_STATE64_COUNT;

	memset(&thread_state, 0x00, sizeof(thread_state));
	thread_state.__rip	= entry + offset;

	src.write((char *)&thread, sizeof(struct thread_command));
	src.write((char *)&flavor, sizeof(uint32_t));
	src.write((char *)&count,  sizeof(uint32_t));
	src.write((char *)&thread_state, sizeof(x86_thread_state64_t));
}

static void dyld_info (std::fstream &src) {
	struct dyld_info_command dyld;

	dyld.cmd			= LC_DYLD_INFO_ONLY;
	dyld.cmdsize		= sizeof(struct dyld_info_command);
	dyld.rebase_off		= 0x0;
	dyld.rebase_size	= 0x0;
	dyld.bind_off		= 0x0;
	dyld.bind_size		= 0x0;
	dyld.weak_bind_off	= 0x0;
	dyld.weak_bind_size	= 0x0;
	dyld.lazy_bind_off	= 0x0;
	dyld.lazy_bind_size	= 0x0;
	dyld.export_off		= 0x0;
	dyld.export_size	= 0x0;

	src.write((char *)&dyld, sizeof(struct dyld_info_command));
}

static void seg_symtab(std::fstream &src, std::vector<std::string> symb) {
	off_t offset = 0x1000;	/* Don't know why need large than __LINKEDIT */
	uint32_t strlen = 0;
	struct symtab_command  symtab;

	for (auto it : symb) {
		strlen += it.size() + 1;
	}

	symtab.cmd			= LC_SYMTAB;
	symtab.cmdsize		= sizeof(struct symtab_command);

	symtab.symoff		= offset;
	symtab.nsyms		= symb.size();
	symtab.stroff		= offset + symb.size() * sizeof(nlist_64);
	symtab.strsize		= strlen;

	src.write((char *)&symtab, sizeof(struct symtab_command));
}

static void seg_dysymtab(std::fstream &src) {
	struct dysymtab_command dysymtab;

	dysymtab.cmd			= LC_DYSYMTAB;
	dysymtab.cmdsize		= sizeof(struct dysymtab_command);

	dysymtab.ilocalsym		= 0x0;
	dysymtab.nlocalsym		= 0x0;
	dysymtab.iextdefsym		= 0x0;
	dysymtab.nextdefsym		= 0x0;
	dysymtab.iundefsym		= 0x0;
	dysymtab.nundefsym		= 0x0;
	dysymtab.tocoff			= 0x0;
	dysymtab.ntoc			= 0x0;
	dysymtab.modtaboff		= 0x0;
	dysymtab.nmodtab		= 0x0;
	dysymtab.extrefsymoff	= 0x0;
	dysymtab.nextrefsyms	= 0x0;
	dysymtab.indirectsymoff	= 0x0;
	dysymtab.nindirectsyms	= 0x0;
	dysymtab.extreloff		= 0x0;
	dysymtab.nextrel		= 0x0;
	dysymtab.locreloff		= 0x0;
	dysymtab.nlocrel		= 0x0;

	src.write((char *)&dysymtab, sizeof(struct dysymtab_command));
}

#define DYDL	"/usr/lib/dyld\00\00\x00\x00\x00\x00"
static void dyld_link (std::fstream &src) {
	struct dylinker_command dylink;

	dylink.cmd			= LC_LOAD_DYLINKER;
	dylink.cmdsize		= sizeof(struct dylinker_command) + sizeof(DYDL);
	dylink.name.offset	= 0x0C;

	src.write((char *)&dylink, sizeof(struct dylinker_command));
	src.write(DYDL, sizeof(DYDL));
}
