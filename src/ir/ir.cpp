/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <stdlib.h>
#include "zerg.h"

IR::IR(std::string dst, Args &args) : Zasm(dst, args) {
	this->_syscall_nr_	= 0;
	this->_lineno_		= 0;
	this->_args_		= args;
}
IR::~IR(void) {
	if (!this->_args_.only_ir) {
		Zasm::dump(this->_args_.entry, this->_args_.symbol);
	}
}

void IR::compile(std::string src) {
	std::string line;
	std::fstream fs(src);

	if (!fs.is_open()) {
		/* cannot open the source code */
		_D(LOG_CRIT, "source file `%s` does NOT exist", src.c_str());
	}

	while (std::getline(fs, line)) {
		unsigned int pos, nr = 0;
		std::string irs[5] = {""}, tmp;
		/* OPCODE DST SRC SIZE INDEX */

		/* lexer IR line-by-line */
		_D(LOG_DEBUG_LEXER, "IR lexer - %s L#%d", line.c_str(), _lineno_);
		for (unsigned cur = 0; cur <= line.size(); ++cur) {
			switch(line[cur]) {
				case '\0': case '#':
					/* Need not process anymore */
					_lineno_ ++;
					goto EMIT;
				case ' ': case '\t':
					/* whitespace */
					break;
				case '"': case '\'':			/* String */
					tmp = line[cur++];
					while ('\0' != line[cur]) {
						tmp += line[cur];

						if (tmp[0] == line[cur]) break;
						++cur;
					}

					if (tmp[0] != line[cur]) {
						/* invalid syntax for string */
						_D(LOG_CRIT, "syntax error - %s", line.c_str());
					}
					irs[nr++] = tmp;
					break;
				default:
					for (pos = cur+1; pos <= line.size(); ++pos) {
						if (' ' == line[pos] || '\t' == line[pos]) break;
					}

					if (nr > ARRAY_SIZE(irs)) {
						_D(LOG_CRIT, "Invalid IR `%s`", line.c_str());
						break;
					}
					irs[nr++] = line.substr(cur, pos-cur);
					cur       = pos;
					break;
			}
		}

		EMIT:
			if (0 != nr) IR::emit(irs[0], irs[1], irs[2], irs[3], irs[4]);
	}
}
IROP IR::opcode(std::string src) {
	IROP opcode = IR_UNKNOWN;

	for (auto it : IROP_map) {
		if (it.first == src) opcode = it.second;
	}

	_D(LOG_DEBUG, "%s -> #%X", src.c_str(), opcode);
	return opcode;
}
void IR::emit(STRING op, STRING _dst, STRING _src, STRING size, STRING index) {
	IROP opcode = IR::opcode(op);

	_D(LOG_DEBUG_IR, "emit %s %s %s %s", op.c_str(), _dst.c_str(),
											_src.c_str(), size.c_str());
	this->emit(opcode, _dst, _src, size, index);
}
std::string IR::tmpreg(void) {
	std::vector<std::string> regs = { REGISTERS };

	ALERT(2 > this->_alloc_regs_.size());
	return this->_alloc_regs_[1];
}
std::string IR::randstr(unsigned int size, std::string prefix) {
	/* generated a random label string */
	const char CH_POOL[] = {	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
								'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
								'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
								'U', 'V', 'W', 'X', 'Y', 'Z'};
	std::string ret = prefix;

	ALERT(ret.size() >  size);
	srand(time(NULL));
	while (ret.size() < size) {
		ret = ret + CH_POOL[random() % ARRAY_SIZE(CH_POOL)];
	}

	return ret;
}

IRType IR::token(std::string src) {
	int cnt = 0;
	IRType type = IR_TOKEN_UNKNOWN;

	if (1 == sscanf(src.c_str(), __IR_REG_FMT__, &cnt) || __IR_SYSCALL_REG__ == src) {
		/* register */
		type = IR_TOKEN_REGISTER;
	} else if (__IR_REFERENCE__ == src.substr(0, 1)) {
		/* reference */
		type = IR_TOKEN_REF;
	} else if ("0x" == src.substr(0, 2) || ('0' <= src[0] && '9' >= src[0])) {
		/* digit */
		type = IR_TOKEN_INT;
	} else if ('\'' == src[0] || '\"' == src[0]) {
		/* string */
		type = IR_TOKEN_STRING;
	} else if ("" != src) {
		/* variable */
		type = IR_TOKEN_VAR;
	}

	_D(LOG_DEBUG_IR, "%-12s -> %d", src.c_str(), type);
	return type;
}
void IR::localvar(std::string var) {
	/* save local variable */
	if (_locals_.end() == std::find(_locals_.begin(), _locals_.end(), var)) {
		this->_locals_.push_back(var);
	}
}
void IR::localvar_reset(void) {
	/* clear-up all local variable */
	this->_locals_.clear();
}
size_t IR::localvar_len(void) {
	/* number of local variable */
	return this->_locals_.size();
}
