/* Copyright (C) 2014-2017 cmj. All right reserved. */
#ifndef __ZASM_INSTRUCTION_H__
#  define __ZASM_INSTRUCTION_H__

#ifdef __x86_64__
#  define MAX_INSTRUCTION_LEN	16
#  include "zasm/x86_64_inst.h"
#else
#  error "Unknown CPU Architecture"
#endif /* __x86_64__ */

#include <fstream>
class Instruction {
	public:
		Instruction(std::string cmd, std::string op1="", std::string op2="");
		virtual ~Instruction() {};

		bool readdressable(void);
		bool isLabel(void);
		off_t setIMM(off_t imm, int size, bool reset=false);
		off_t setIMM(std::string imm, int size, bool reset=false);
		off_t length(void);

		std::string label(void);
		std::string refer(void);
		std::string show(void);

		virtual void assemble(void);

		Instruction& operator << (std::fstream &dst);
	private:
		off_t _length_;
		unsigned char _payload_[MAX_INSTRUCTION_LEN];
		std::string _label_;

		ZasmToken& cmd(void);
		ZasmToken& dst(void);
		ZasmToken& src(void);

		off_t offset(void);
		std::vector<ZasmToken *>_inst_;

	#ifdef __x86_64__
		void legacyPrefix(X86_64_INST &inst);
		void opcode(X86_64_INST &inst);
		void modRW(X86_64_INST &inst);
		void displacement(X86_64_INST &inst);
		void immediate(X86_64_INST &inst);
	#endif /* __x86_64__ */
};

#endif /* __ZASM_INSTRUCTION_H__ */
