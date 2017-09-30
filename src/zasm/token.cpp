/* Copyright (C) 2014-2016 cmj. All right reserved. */

#include <iostream>
#include <sstream>
#include <algorithm>

#include "zasm.h"


InstToken::InstToken(std::string src) : _src_(src) {
	unsigned int start = 0, end = 0;
	std::string tmp;
	InstToken *token = NULL;

	/* classify the token */
	if (!*this) {
		_D(LOG_CRIT, "empty string");
	} else if (this->isREG()) {	/* REGISTER */
		const std::vector<std::string> regs8  = { X86_REG64, X86_EXTREG64 };
		const std::vector<std::string> regs16 = { X86_REG32, X86_EXTREG32 };
		const std::vector<std::string> regs32 = { X86_REG16, X86_EXTREG16 };
		const std::vector<std::string> regs64 = { X86_REG8, X86_REG_LOWER8, X86_EXTREG8 };

		_based_  = this;
		_index_  = NULL;
		_offset_ = NULL;

		if (regs8.end() != std::find(regs8.begin(), regs8.end(), this->_src_))  {
			_size_ = CPU_8BIT;
		} else if (regs16.end() != std::find(regs16.begin(), regs16.end(), this->_src_)) {
			_size_ = CPU_16BIT;
		} else if (regs32.end() != std::find(regs32.begin(), regs32.end(), this->_src_)) {
			_size_ = CPU_32BIT;
		} else if (regs64.end() != std::find(regs64.begin(), regs64.end(), this->_src_)) {
			_size_ = CPU_64BIT;
		} else {
			_D(LOG_CRIT, "Unkonwn register - `%s`", _src_.c_str());
		}
	} else if (this->isIMM()) {	/* IMMEDIATE */
		if (this->isRANImm()) {
			tmp = this->_src_.substr(0, this->_src_.find(ZASM_RANGE));
			this->_based_ = new InstToken(tmp);

			tmp = this->_src_.substr(this->_src_.find(ZASM_RANGE)+1);
			this->_index_ = new InstToken(tmp);
		} else {
			_based_  = this;
			_index_  = NULL;
			_offset_ = NULL;
		}

		off_t len = this->_based_->asInt();

		if (0 == (len & ~0xFF)) {
			_size_ = CPU_8BIT;
		} else if (0 == (len & ~0xFFFF)) {
			_size_ = CPU_16BIT;
		} else if (0 == (len & ~0xFFFFFFFF)) {
			_size_ = CPU_32BIT;
		} else {
			_size_ = CPU_64BIT;
		}
	} else if (this->isREF()) { /* REFERENCE */
		_based_  = this;
		_index_  = NULL;
		_offset_ = NULL;
		_size_   = CPU_UNKNOWN;	/* undefined size */
	} else {					/* MEMORY */
		if (0 == this->_src_.find(ZASM_MEM_QWORD)) {
			this->_size_ = CPU_64BIT;
			tmp = this->_src_.substr(strlen(ZASM_MEM_QWORD));
		} else if (0 == this->_src_.find(ZASM_MEM_DWORD)) {
			this->_size_ = CPU_32BIT;
			tmp = this->_src_.substr(strlen(ZASM_MEM_DWORD));
		} else if (0 == this->_src_.find(ZASM_MEM_WORD)) {
			this->_size_ = CPU_16BIT;
			tmp = this->_src_.substr(strlen(ZASM_MEM_WORD));
		} else if (0 == this->_src_.find(ZASM_MEM_BYTE)) {
			this->_size_ = CPU_8BIT;
			tmp = this->_src_.substr(strlen(ZASM_MEM_BYTE));
		} else {
			this->_size_ = CPU_64BIT;
			tmp = this->_src_;
		}

		tmp   = strip(tmp);
		while (']' != tmp[end]) {
			if ('[' != tmp[0] || ']' != tmp[tmp.size()-1]) {
				_D(LOG_CRIT, "Unknown token - `%s`", this->_src_.c_str());
			}

			for (start = 0; start < tmp.size(); ++start) {
				if (' ' != tmp[start]) break;
			}
			for (end = start+1; end < tmp.size(); ++end) {
				if (' ' == tmp[end] || ']' == tmp[end]) break;
				if ('+' == tmp[end] || '-' == tmp[end] || ':' == tmp[end]) break;
			}
			token = new InstToken(tmp.substr(start, end-1));
			if (token->isIMM()) {
				if (NULL != this->_offset_) _D(LOG_CRIT, "Multiple offset in memory");
				this->_offset_ = token;
			} else if (NULL == this->_based_) {
				this->_based_ = token;
			} else if (NULL == this->_index_) {
				this->_index_ = token;
			} else {
				_D(LOG_CRIT, "Unknown token - `%s`", this->_src_.c_str());
			}

			start = end+1;
		}
	}
}


/* ======== simple token classify ======== */
bool InstToken::isREG(void) {
	const std::vector<std::string> regs = { REGISTERS };
	unsigned int idx = std::find(regs.begin(), regs.end(), this->_src_) - regs.begin();

	return idx != regs.size();
}
bool InstToken::isMEM(void) {
	bool blRet = false;

	if (true == (blRet = this->isREF())) {
		_D(LOG_ZASM_DEBUG, "treat reference as memory space");
		goto END;
	} else if ("" != this->_src_ && '[' == this->_src_[0]) {
		_D(LOG_ZASM_DEBUG, "simple memory space without size specified");
		_size_ = CPU_64BIT;
		blRet  = true;
		goto END;
	} else if (0 == this->_src_.find(ZASM_MEM_BYTE)  && '[' == this->_src_[5]) {
		_size_ = CPU_8BIT;
		blRet  = true;
		goto END;
	} else if (0 == this->_src_.find(ZASM_MEM_WORD)  && '[' == this->_src_[5]) {
		_size_ = CPU_16BIT;
		blRet  = true;
		goto END;
	} else if (0 == this->_src_.find(ZASM_MEM_DWORD) && '[' == this->_src_[6]) {
		_size_ = CPU_32BIT;
		blRet  = true;
		goto END;
	} else if (0 == this->_src_.find(ZASM_MEM_QWORD) && '[' == this->_src_[6]) {
		_size_ = CPU_64BIT;
		blRet  = true;
		goto END;
	}
END:
	return blRet;
}
bool InstToken::isIMM(void) {
	bool blRet = false;

	if (this->isRANImm()) {
		blRet = true;
	} else if ("0x" == this->_src_.substr(0, 2) || "0X" == this->_src_.substr(0, 2)) {
		for (unsigned int idx = 2; idx < this->_src_.size(); ++idx) {
			if ('0' <= this->_src_[idx] && this->_src_[idx] <= '9') {
				continue;
			} else if ('a' <= (this->_src_[idx] | 0x20) && (this->_src_[idx] | 0x20) <= 'f') {
				continue;
			}
			goto END;
		}
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
bool InstToken::isREF(void) {
	return	ZASM_REFERENCE == _src_[0] ||
			ZASM_CURRENT_POS == _src_ || ZASM_SESSION_POS == _src_;
}

/* ================ */
bool InstToken::isLOWReg(void) {
	InstToken *based = this->_based_;
	const std::vector<std::string> low = { X86_REG_LOWER8 };

	return NULL != based && IN_VECTOR(based->_src_, low);
}
bool InstToken::isPOSREG(void) {
	InstToken *based = this->_based_;

	return NULL != based && (4 == based->asInt() % 8 || 5 == based->asInt() % 8);
}
bool InstToken::isSEGReg(void) {
	InstToken *based = this->_based_;
	const std::vector<std::string> seg = { SEGREGS };

	return NULL != based && IN_VECTOR(based->_src_, seg);
}
bool InstToken::isEXT(void) {
	InstToken *based = this->_based_;
	const std::vector<std::string> ext = { X86_EXTREGS };

	return NULL != based && IN_VECTOR(based->_src_, ext);
}

std::string InstToken::unescape(void) {
	std::string dst = "";
	char tmp = 0x0;

	ALERT(('"' != this->_src_[0] && '\'' != this->_src_[0]) ||
			this->_src_[0] != this->_src_[this->_src_.size()-1]);
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
