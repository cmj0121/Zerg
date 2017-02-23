/* Copyright (C) 2014-2016 cmj. All right reserved. */
#ifndef __ZASM_H__
#  define __ZASM_H__

#include "utils.h"

#define TOKEN_ENTRY         "ENTRY"
#define TOKEN_ASM           "asm"
#define ZASM_INCLUDE		"include"
#define ZASM_MEM_BYTE		"byte"
#define ZASM_MEM_WORD		"word"
#define ZASM_MEM_DWORD		"dword"
#define ZASM_MEM_QWORD		"qword"


#ifdef __x86_64__
#  define MAX_INSTRUCTION_LEN	16
#  include "zasm/x86_64_inst.h"
#endif /* __x86_64__ */

class Utils {
	public:
		virtual ~Utils();

		bool  isInt(std::string src);
		off_t toInt(std::string src);

		std::string unescape(std::string src);
		std::string toHex(unsigned char *payload, size_t size);
};
class ZasmToken {
	public:
		ZasmToken() : _src_("") {};
		ZasmToken(std::string src);

		bool isREG(void);
		bool isMEM(void);
		bool isMEM2(void);	/* Memory with two registers */
		bool isIMM(void);
		bool isEXT(void);
		bool isREF(void);
		bool isSSE(void);	/* Streaming SIMD Extensions */

		ZasmToken* asReg(void);
		ZasmToken* indexReg(void);
		off_t asInt(void);
		off_t offset(void);

		int size(void);

		std::string raw(void);
		std::string unescape(void);

		bool match(unsigned int flag);
		bool operator== (std::string src);
		bool operator!= (std::string src);
		operator int() const;
	private:
		std::string _src_;
};
static ZasmToken EMPTY_TOKEN("");

#include <fstream>
class Instruction {
	public:
		Instruction(ZasmToken *cmd, ZasmToken *op1 = NULL, ZasmToken *op2=NULL);
		Instruction(std::string cmd, std::string op1="", std::string op2="");
		virtual ~Instruction() {};

		bool readdressable(void);
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

#include <vector>
class Binary : public Utils {
	public:
		Binary(std::string src, bool pie=false);
		virtual ~Binary();

		/* binary-specified function */
		off_t dump(off_t entry = 0x1000, bool symb=false);

		void reallocreg(void);
		off_t length(void);
		off_t nrInst(void);
		void insert(Instruction* inst, int pos);
		Instruction *getInst(int pos);
		std::string get(int pos);
		Binary& operator+= (Instruction *inst);
	private:
		bool _pie_;
		std::string  _src_;
		std::fstream _bin_;
		std::vector<Instruction *> _inst_;
		std::vector<std::string> _symb_;
};

class Zasm : public Binary {
	public:
		Zasm(std::string src, bool pie=false) : Binary(src, pie) {};
		void compile(std::fstream &src, bool symb=false);
		ZasmToken* token(std::fstream &src);
};

#endif /* __ZASM_H__ */

