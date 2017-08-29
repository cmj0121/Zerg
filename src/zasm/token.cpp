/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iostream>
#include <sstream>
#include <algorithm>
#include "zasm.h"

#ifndef REGISTERS
#   error "You need define REGISTERS first"
#endif /* REGISTERS */

bool InstToken::isNULL(void) {
	return "" == this->_src_;
}

bool InstToken::isREG(void) {
	const std::vector<std::string> regs = { REGISTERS };
	unsigned int idx = std::find(regs.begin(), regs.end(), this->_src_) - regs.begin();

	return idx != regs.size() || this->isSSE() || this->isSegREG();
}
bool InstToken::isSegREG(void) {
	const std::vector<std::string> regs = { SEGMENT_REG };
	unsigned int idx = std::find(regs.begin(), regs.end(), this->_src_) - regs.begin();

	return idx != regs.size();
}
bool InstToken::isPosREG(void) {
	return this->isREG() && (4 == this->asInt() % 8 || 5 == this->asInt() % 8);
}
bool InstToken::isMEM(void) {
	bool blRet = false;

	if (true == (blRet = this->isREF())) {
		_D(LOG_ZASM_DEBUG, "treat reference as memory space");
		goto END;
	} else if ("" != this->_src_ && '[' == this->_src_[0]) {
		_D(LOG_ZASM_DEBUG, "simple memory space without size specified");
		blRet = true;
		goto END;
	} else if ( (0 == this->_src_.find(ZASM_MEM_BYTE)  && '[' == this->_src_[5]) ||
				(0 == this->_src_.find(ZASM_MEM_WORD)  && '[' == this->_src_[5]) ||
				(0 == this->_src_.find(ZASM_MEM_DWORD) && '[' == this->_src_[6]) ||
				(0 == this->_src_.find(ZASM_MEM_QWORD) && '[' == this->_src_[6])) {
		_D(LOG_ZASM_DEBUG, "simple memory space with size specified");
		blRet = true;
		goto END;
	}
END:
	return blRet;
}
bool InstToken::isMEM2(void) {
	InstToken *token = NULL;

	if (!this->isMEM() || NULL == (token = this->indexReg())) {
		return false;
	}
	delete token;
	return true;
}
bool InstToken::isIMM(void) {
	bool blRet = false;

	if (*this == "") {
		goto END;
	} else if ("0x" == this->_src_.substr(0, 2) || "0X" == this->_src_.substr(0, 2)) {
		for (unsigned int idx = 2; idx < this->_src_.size(); ++idx) {
			if ('0' <= this->_src_[idx] && this->_src_[idx] <= '9') {
				continue;
			} else if ('a' <= (this->_src_[idx] | 0x20) && (this->_src_[idx] | 0x20) <= 'f') {
				continue;
			}
			goto END;
		}
	} else if (this->isREF()) {
		blRet = true;
	} else {
		for (unsigned int idx = 0; idx < this->_src_.size(); ++idx) {
			if ('0' <= this->_src_[idx] && this->_src_[idx] <= '9') {
				continue;
			}
			goto END;
		}
	}
	blRet = true;
END:
	return blRet;
}
bool InstToken::isEXT(void) {
	if (this->isREF()) {
		return false;
	} else if (this->isREG()) {
		unsigned int idx = 0;
		const std::vector<std::string> regs = { REG_EXTENSION };

		idx = std::find(regs.begin(), regs.end(), this->_src_) - regs.begin();
		return idx != regs.size();
	} else if (this->isMEM()) {
		InstToken *tmp = this->asReg();
		return tmp->isEXT();
	}
	return false;
}
bool InstToken::isREF(void) {
	/* Check is reference or not */
	return ZASM_REFERENCE == this->_src_[0] || ZASM_CURRENT_POS == this->_src_ ||
		 ZASM_SESSION_POS == this->_src_;
}
bool InstToken::isSSE(void) {
	/* Streaming SIMD Extensions Register */

	return 0 == this->_src_.find("xmm");
}
bool InstToken::isDecorator(void) {
	/* return the is decorator or NOT */
	return ZASM_DECORATOR == this->_src_[0];
}

off_t InstToken::asInt(void) {
	if (0 < this->_src_.size() && this->isREF()) {
		return (off_t)-1;
	} else if (this->isSSE()) {
		std::string off = this->_src_.substr(3);

		return atoi(off.c_str());
	} else if (this->isSegREG()) {
		const std::vector<std::string> regs = { SEGMENT_REG };
		int idx = 0;

		idx = std::find(regs.begin(), regs.end(), this->raw()) - regs.begin();
		return idx;
	} else if (this->isREG() || this->isMEM()) {
		const std::vector<std::string> regs = { REGISTERS };
		int idx = 0;

		idx = (std::find(regs.begin(), regs.end(), this->asReg()->raw()) - regs.begin()) % 8;
		return idx;
	} else if (this->isIMM()) {
		std::stringstream ss;
		off_t ret;

		if ("0x" == _src_.substr(0, 2) || "0X" == _src_.substr(0, 2)) {
			ss << std::hex << this->_src_;
		} else {
			ss << this->_src_;
		}
		ss >> ret;
		return ret;
	} else {
		_D(LOG_CRIT, "Not Implemented '%s'", this->_src_.c_str());
		exit(-1);
	}
}
InstToken* InstToken::asReg(void) {
	if (!(this->isREG() || this->isMEM())) {
		_D(LOG_CRIT, "Only support register or member");
		exit(-1);
	} else if (this->isREG()) {
		return this;
	} else {
		unsigned int start, end;

		for (start = 0; start < _src_.size(); ++start) {
			if ('[' == _src_[start]) break;
		}
		start ++;
		for (; start < _src_.size(); ++start) {
			if (' ' != _src_[start]) break;
		}
		for (end = start+1; end < _src_.size(); ++end) {
			if (' ' == _src_[end] || ']' == _src_[end]) break;
			if ('+' == _src_[end] || '-' == _src_[end]) break;
		}
		return new InstToken(_src_.substr(start, end-start));
	}
}
InstToken* InstToken::indexReg(void) {
	std::string substr;
	InstToken *token = NULL, *tmpToken = NULL;

	for (unsigned int s = 0; s < _src_.size(); ++s) {
		if ('[' != _src_[s]) continue;

		for (unsigned int e = s+1; e < _src_.size(); ++e) {
			if (']' != this->_src_[e]) continue;
			substr = this->_src_.substr(s+1, e-s-1);
			break;
		}
		break;
	}

	if ("" != substr) {
		unsigned int cnt = 0, idx, pos;
		std::string tmp;

		for (idx = 0, pos = 0; idx < substr.size(); ++idx) {
			switch (substr[idx]) {
				case '+': case '-': case ' ':
					tmpToken = new InstToken(substr.substr(pos, idx-pos));
					pos = idx+1;
					if (tmpToken->isREG()) {
						token = tmpToken;
						cnt ++;
					} else {
						delete tmpToken;
						tmpToken = NULL;
					}
					break;
				default:
					break;
			}
		}
		tmpToken = new InstToken(substr.substr(pos, idx-pos));
		if (tmpToken->isREG()) {
			token = tmpToken;
			cnt ++;
		} else {
			delete tmpToken;
		}

		if (cnt > 2) {
			_D(LOG_CRIT, "Not support %s", this->_src_.c_str());
			exit(-1);
		}
		if (cnt != 2) {
			delete token;
			token = NULL;
		}
	}
	return token;
}
off_t InstToken::offset(void) {
	off_t offset = 0;

	if (!this->isMEM() || this->isREF()) {
		offset = 0;
	}

	for (unsigned int s = 0, e = 0; s < this->_src_.size(); ++s) {
		int sign = 1;
		if ('+' != this->_src_[s] && '-' != this->_src_[s]) continue;

		if ('-' == this->_src_[s]) sign = -1;

		/* Skip space */
		for (++s; s < this->_src_.size(); ++s) {
			if (' ' == this->_src_[s]) continue;
			break;
		}
		for (e = s+1; e < this->_src_.size() && ']' != this->_src_[e]; ++e) {
			if (' ' == this->_src_[e] || '+' == this->_src_[e] || '-' == this->_src_[e]) break;
		}

		InstToken token = InstToken(this->_src_.substr(s, e-s));

		if (token.isIMM()) {
			offset += token.asInt() * sign;
		}
	}

	_D(LOG_ZASM_DEBUG, "offset %s -> " OFF_T, this->_src_.c_str(), offset);
	return offset;
}

int InstToken::size(void) {
	int size = CPU_UNKNOWN;
	if (this->isREG()) {
		const std::vector<std::string> regs8  = { REG_GENERAL_8,  REG_EXTENSION_8 };
		const std::vector<std::string> regs16 = { REG_GENERAL_16, REG_EXTENSION_16 };
		const std::vector<std::string> regs32 = { REG_GENERAL_32, REG_EXTENSION_32 };
		const std::vector<std::string> regs64 = { REG_GENERAL_64, REG_EXTENSION_64 };

		if (regs8.end() != std::find(regs8.begin(), regs8.end(), this->_src_))  {
			size = CPU_8BIT;
		} else if (regs16.end() != std::find(regs16.begin(), regs16.end(), this->_src_)) {
			size = CPU_16BIT;
		} else if (regs32.end() != std::find(regs32.begin(), regs32.end(), this->_src_)) {
			size = CPU_32BIT;
		} else if (regs64.end() != std::find(regs64.begin(), regs64.end(), this->_src_)) {
			size = CPU_64BIT;
		}
	} else if (this->isREF()) {
		size = CPU_64BIT;
	} else if (this->isMEM()) {
		if (0 == this->_src_.find(ZASM_MEM_BYTE)) {
			size = CPU_8BIT;
		} else if (0 == this->_src_.find(ZASM_MEM_WORD)) {
			size = CPU_16BIT;
		} else if (0 == this->_src_.find(ZASM_MEM_DWORD)) {
			size = CPU_32BIT;
		} else if (0 == this->_src_.find(ZASM_MEM_QWORD)) {
			size = CPU_64BIT;
		} else {
			size = CPU_64BIT;
		}
	} else if (this->isIMM()) {
		off_t len = this->asInt();

		if (0 == (len & ~0x7F)) {
			size = CPU_8BIT;
		} else if (0 == (len & ~0x7FFF)) {
			size = CPU_16BIT;
		} else if (0 == (len & ~0xFFFFFFFF)) {
			size = CPU_32BIT;
		} else {
			size = CPU_64BIT;
		}
	}

	_D(LOG_ZASM_DEBUG, "`%s` size %d", this->_src_.c_str(), size);
	return size;
}
std::string InstToken::raw(void) {
	/* Just return the original string */
	return this->_src_;
}
std::string InstToken::unescape(void) {
	std::string dst = "";
	char tmp = 0x0;

	ALERT('"' != this->_src_[0] || '"' != this->_src_[this->_src_.size()-1]);
	for (unsigned int i = 1; i < this->_src_.size()-1; ++i) {
		if ('\\' == this->_src_[i]) {
			switch (this->_src_[++i]) {
				case 'a':
					dst += '\a';
					break;
				case 'b':
					dst += '\b';
					break;
				case 't':
					dst += '\t';
					break;
				case 'n':
					dst += '\n';
					break;
				case 'v':
					dst += '\v';
					break;
				case 'f':
					dst += '\f';
					break;
				case 'r':
					dst += '\r';
					break;
				case 'x':
					ALERT(i+2 >= this->_src_.size());

					tmp = 0x0;
					for (unsigned int j = i+1; j < i+3; ++j) {
						if ('0' <= this->_src_[j] && '9' >= this->_src_[j]) {
							/* digit */
							tmp = (tmp << 4) + this->_src_[j] - '0';
						} else if ('a' <= this->_src_[j] && 'f' >= this->_src_[j]) {
							/* lower case */
							tmp = (tmp << 4) + this->_src_[j] - 'a' + 0x0A;
						} else if ('A' <= this->_src_[j] && 'F' >= this->_src_[j]) {
							/* upper case */
							tmp = (tmp << 4) + this->_src_[j] - 'A' + 0x0A;
						} else {
							_D(LOG_CRIT, "Not support %c in HEX", this->_src_[j]);
						}
					}

					dst += tmp;
					i += 2;
					break;
				case '\\':
					dst += '\\';
					break;
				default:
					dst += '\\' + this->_src_[i];
					break;
			}
			continue;
		}

		dst += this->_src_[i];
	}

	_D(LOG_ZASM_DEBUG, "escape `%s` -> `%s`", this->_src_.c_str(), dst.c_str());
	return dst;
};
bool InstToken::match(unsigned int flag) {
	bool blRet = false;

	_D(LOG_ZASM_DEBUG, "check %s vs %X", this->_src_.c_str(), flag);
	if (INST_NONE == flag && *this == "") {
		blRet = true;
		goto END;
	}
	if (0 != (INST_REG & flag) && this->isREG() && ! this->isSegREG()) {
		blRet = true;
		goto CHECK_SIZE;
	}
	if (0 != (INST_MEM & flag) && this->isMEM()) {
		blRet = true;
		goto CHECK_SIZE;
	}
	if (0 != (INST_IMM & flag) && (this->isIMM() || this->isREF())) {
		blRet = true;
		if (this->isREF()) goto END;
		goto CHECK_SIZE;
	}

#ifdef __x86_64__
	if (0 != (INST_REG_SEGMEM & flag) && this->isSegREG()) {
		blRet = true;
		goto END;
	}
	if (INST_REG_SPECIFY & flag) {
		switch(flag) {
			case INST_REG_RAX:
				blRet = *this == "rax";
				break;
			case INST_REG_AH:
				blRet = *this == "ah";
				break;
			case INST_REG_AL:
				blRet = *this == "al";
				break;
			case INST_REG_CL:
				blRet = *this == "cl";
				break;
			default:
				_D(LOG_CRIT, "Not Implemented");
				exit(-1);
		}
	}
#endif /* __x86_64__ */
	goto END;

CHECK_SIZE:
	switch (flag & INST_SIZE_ALL) {
		case INST_SIZE8:
			if (this->isREG() || this->isIMM()) {
				blRet = this->size() == 1 && 0 == (~0x7F & this->asInt());
			} else {
				/* FIXME */
				blRet = true;
			}
			break;
		case INST_SIZE16:
			blRet = this->size() <= 2;
			break;
		case INST_SIZE32:
			blRet = this->size() <= 4;
			break;
		case INST_SIZE16_32:
			blRet = this->size() <= 4;
			break;
		case INST_SIZE16_32_64:
			blRet = this->isIMM() ||  1 != this->size();
			break;
		case INST_SIZE64:
			blRet = this->size() <= 8;
			break;
		case INST_SIZE_ALL:
		default:
			blRet = true;
			break;
	}
END:
	return blRet;
}
bool InstToken::operator== (std::string src) {
	/* Compared with string */
	return src == this->_src_;
}
bool InstToken::operator!= (std::string src) {
	/* Compared with string */
	return ! (*this == src);
}
InstToken::operator int() const {
	return this->_src_ != "";
}
