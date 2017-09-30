/* Copyright (C) 2014-2017 cmj. All right reserved. */
#ifndef __ZASM_INSTRUCTION_H__
#  define __ZASM_INSTRUCTION_H__

#define CPU_UNKNOWN	0
#define CPU_8BIT	1
#define CPU_16BIT	2
#define CPU_32BIT	4
#define CPU_64BIT	8

#define INST_NONE			0x0000
#define INST_REG			0x0001
#define INST_MEM			0x0002
#define INST_IMM			0x0004
#define INST_REF			0x0008

#define INST_SIZE8			0x0010
#define INST_SIZE16			0x0020
#define INST_SIZE32			0x0040
#define INST_SIZE64			0x0080
#define INST_SIZE128		0x0100
#define INST_SIZE256		0x0200
#define INST_SIZE512		0x0400
#define INST_SIZE_ALL		0x0FF0

#define INST_CONST			0x1000
#define INST_EXTRA_SHT		16

#ifdef __x86_64__
#  define MAX_INSTRUCTION_LEN	16
#  include "zasm/x86_64_inst.h"
#else
#  error "Unknown CPU Architecture"
#endif /* __x86_64__ */

class InstToken {
	public:
		InstToken(std::string src="");

		/* simple token classify */
		bool isREG(void);			/* register   */
		bool isMEM(void);			/* memory     */
		bool isIMM(void);			/* immediate  */
		bool isREF(void);			/* referenced */

		bool isLOWReg(void);		/* lower 8-bit register */
		bool isPOSREG(void);		/* special register - position-related */
		bool isSEGReg(void);		/* segment register */
		bool isRANImm(void)			{ return _src_.size() != _src_.find(ZASM_RANGE); }
		bool isEXT(void);			/* extension in 64-bit mode */

		/* [ based + index + offset ] */
		InstToken* asReg(void)		{ return this->_based_;  }
		InstToken* idxReg(void)		{ return this->_index_;  }
		InstToken* offReg(void)		{ return this->_offset_; }
		/* the size of this token, CPU_8BIT ~ CPU_64BIT */
		int size(void)				{ return this->_size_;   }
		/* treated as integer */
		off_t asInt(void)			{ return this->_int_;    }

		std::string unescape(void);	/* unescape string */

		bool operator == (std::string src)	{ return _src_ == src; }
		bool operator != (std::string src)	{ return _src_ != src; }
		operator int(void) const 			{ return _src_ != "";  }
	private:
		std::string _src_;
		off_t _size_, _int_;
		InstToken *_based_, *_index_, *_offset_;
};

#include <fstream>
class Instruction {
	public:
		Instruction(STRING cmd, STRING op1="", STRING op2="", int mode=DEFAULT_MODE);
		virtual ~Instruction() {};

		off_t setIMM(off_t imm, int size, bool reset=false);
		off_t setRepeat(off_t imm);
		off_t length(void);								/* length of this instruction */

		std::string label(void);						/* symbol */
		std::string refer(void);						/* referenced symbol */

		void assemble(Instruction &inst, int mode=DEFAULT_MODE);
		friend std::fstream& operator << (const std::fstream &dst, Instruction &inst);
	private:
		bool _absaddr_;
		off_t _length_, _repeat_;
		unsigned char _payload_[MAX_INSTRUCTION_LEN];
		std::string _label_;
		InstToken cmd, dst, src;
};

#endif /* __ZASM_INSTRUCTION_H__ */
