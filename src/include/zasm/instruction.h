/* Copyright (C) 2014-2017 cmj. All right reserved. */
#ifndef __ZASM_INSTRUCTION_H__
#  define __ZASM_INSTRUCTION_H__

#ifdef __x86_64__
#  define MAX_INSTRUCTION_LEN	16
#  include "zasm/x86_64_inst.h"
#else
#  error "Unknown CPU Architecture"
#endif /* __x86_64__ */

class InstToken {
	public:
		InstToken(std::string src="") : _src_(src) {};

		bool isREG(void);			/* register */
		bool isPosREG(void);		/* special register - position-related */
		bool isMEM(void);			/* memory */
		bool isMEM2(void);			/* memory with two registers */
		bool isIMM(void);			/* immediate */
		bool isEXT(void);			/* extension in 64-bit mode */
		bool isREF(void);			/* referenced symbol */
		bool isSSE(void);			/* streaming SIMD extensions */

		InstToken* asReg(void);		/* treated as register */
		InstToken* indexReg(void);	/* index register of memory */
		off_t asInt(void);			/* treated as integer */
		off_t offset(void);			/* offset of memory */

		int size(void);

		std::string raw(void);		/* row string */
		std::string unescape(void);	/* unescape string */

		bool match(unsigned int flag);
		bool operator== (std::string src);
		bool operator!= (std::string src);
		operator int() const;
	private:
		std::string _src_;
};

#include <fstream>
class Instruction {
	public:
		Instruction(std::string cmd, std::string op1="", std::string op2="");
		virtual ~Instruction() {};

		bool readdressable(void);
		bool isShowLabel(void);							/* show the symbol or not */
		off_t setIMM(off_t imm, int size, bool reset=false);
		off_t length(void);								/* length of this instruction */
		off_t offset(void);								/* memory offset */

		std::string label(void);						/* symbol */
		std::string refer(void);						/* referenced symbol */

		virtual void assemble(void);

		Instruction& operator << (std::fstream &dst);
	private:
		off_t _length_;
		unsigned char _payload_[MAX_INSTRUCTION_LEN];
		std::string _label_;
		InstToken cmd, dst, src;


	#ifdef __x86_64__
		void legacyPrefix(X86_64_INST &inst);
		void opcode(X86_64_INST &inst);
		void modRW(X86_64_INST &inst);
		void displacement(X86_64_INST &inst);
		void immediate(X86_64_INST &inst);
	#endif /* __x86_64__ */
};

#endif /* __ZASM_INSTRUCTION_H__ */
