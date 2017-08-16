/* Copyright (C) 2014-2017 cmj <cmj@cmj.tw>. All right reserved. */

#include <iomanip>
#include "zerg.h"

/* Private Properties */
ZergToken Parser::lexer(void) {
	int base=10;
	size_t pos = 0;
	std::string token;
	ZType type = ZTYPE_UNKNOWN;

	static bool blNewline = false, blEOF = false, blEOL = false;
	static int _indent_cnt_ = 0, _indent_cur_ = 0;;
	static std::string _linebuf_, _indent_word_;

	/* First, handle the indent/dedent case */
	if (_indent_cur_ != _indent_cnt_) goto INDENT_PROCESSOR;

	/* lexer */
LEXER:
	do {
		/* flush the line from fp and store on _linebuf_ */
		while (0 == _linebuf_.size()) {
			this->_lineno_ ++;

			_D(LOG_DEBUG_LEXER, "reload the line buffer");
			if (std::getline(this->_fp_, _linebuf_)) {
				_D(LOG_DEBUG_LEXER, "parse the line - #%-4d %s", _lineno_, _linebuf_.c_str());
				token = "[NEWLINE]";
				type  = ZTYPE_NEWLINE;
			} else {
				/* EOF - reset all indent */
				_indent_cur_ = 0;

				if (!blEOF) {
					blEOF = true;
					token = "[NEWLINE]";
					type  = ZTYPE_NEWLINE;
					goto END_LEXER;
				} else if (_indent_cur_ != _indent_cnt_) {
					/* clean-up the indent */
					goto INDENT_PROCESSOR;
				} else if (false == blEOL) {
					blEOL = true;
					token = "[NEWLINE]";
					type  = ZTYPE_NEWLINE;
					goto END_LEXER;
				}
			}

			goto END_LEXER;
		}

		/* simple process the indent case when newline */
		if (blNewline && !ISWHITESPACE(_linebuf_[0])) {
			_D(LOG_DEBUG_LEXER, "new line without any indent `%s`", _linebuf_.c_str());
			_indent_cur_ = 0;
			if (_indent_cur_ != _indent_cnt_) goto INDENT_PROCESSOR;
		}

		_D(LOG_DEBUG_LEXER, "lexer on line - `%s`", _linebuf_.c_str());
		switch(_linebuf_[0]) {
			case '#': case '\0':						/* End-of-Line*/
				token = "";
				_linebuf_ = "";
				continue;
			case ' ': case '\t':						/* white-space */
				for (pos = 0; pos < _linebuf_.size(); ++pos) {
					/* need NOT process the indent/dedent case */
					if (!(_linebuf_[pos] == ' ' || _linebuf_[pos] == '\t')) break;
				}
				token = _linebuf_.substr(0, pos);

				if ('\0' == _linebuf_[pos]) {	/* all white-space */
					_D(LOG_DEBUG_LEXER, "all white-space");

					_linebuf_ = "";
					continue;
				} else if (blNewline) {			/* process the current indent status */
					if(0 == _indent_word_.size()) {
						_D(LOG_DEBUG_LEXER, "new indent detected");
						_indent_word_ = _linebuf_.substr(0, pos);
						_linebuf_     = _linebuf_.substr(pos);

						_indent_cur_ ++;
						goto INDENT_PROCESSOR;
					}

					_indent_cur_ = 0;
					while (_indent_word_ == token.substr(0, _indent_word_.size())) {
						_indent_cur_ ++;
						token = token.substr(_indent_word_.size());
					}

					if (0 != token.size()) _D(LOG_CRIT, "indent error");
					_linebuf_ = _linebuf_.substr(pos);

					if (_indent_cur_ != _indent_cnt_) goto INDENT_PROCESSOR;

					blNewline = false;
					continue;
				}

				_linebuf_ = _linebuf_.substr(pos);
				continue;
			case '1': case '2': case '3': case '4':		/* number */
			case '5': case '6': case '7': case '8':
			case '9': case '0':
				for (pos = 1; pos <= _linebuf_.size(); ++pos) {
					if (10 == base && _linebuf_[pos] >= '0' && _linebuf_[pos] <= '9') {
						/* NUMBER */
						continue;
					} else if (2 == base && _linebuf_[pos] >= '0' && _linebuf_[pos] <= '1') {
						/* NUMBER */
						continue;
					} else if (8 == base && _linebuf_[pos] >= '0' && _linebuf_[pos] <= '7') {
						/* NUMBER */
						continue;
					} else if (16 == base && ISHEX(_linebuf_[pos])) {
						/* NUMBER */
						continue;
					} else if (pos == 1) {
						if (_linebuf_[pos] == 'x' || _linebuf_[pos] == 'X') {
							base = 16;
							continue;
						} else if (_linebuf_[pos] == 'b' || _linebuf_[pos] == 'B') {
							base = 2;
							continue;
						} else if (_linebuf_[pos] == 'o' || _linebuf_[pos] == 'O') {
							base = 8;
							continue;
						} else if (_linebuf_[pos] == '.') {
							/* FLOAT */
							continue;
						}
					}
					break;
				}
				token     = _linebuf_.substr(0, pos);
				type      = ZTYPE_NUMBER;
				_linebuf_ = _linebuf_.substr(pos);
				goto END_LEXER;
				break;
			case '+': case '-':	case '>': case '<':		/* OPERATOR */
			case '*': case '/': case '%': case '~':
				pos = 1;
				while (_linebuf_[pos] == _linebuf_[0]) pos++;

				/* HACK */
				if ('-' == _linebuf_[0] && '>' == _linebuf_[1]) pos ++;

				token     = _linebuf_.substr(0, pos);
				_linebuf_ = _linebuf_.substr(pos);

				     if (token == "+")  { type = ZTYPE_ADD;     }
				else if (token == "-")  { type = ZTYPE_SUB;     }
				else if (token == "*")  { type = ZTYPE_MUL;     }
				else if (token == "/")  { type = ZTYPE_DIV;     }
				else if (token == "%")  { type = ZTYPE_MOD;     }
				else if (token == "~")  { type = ZTYPE_LIKE;    }
				else if (token == "<")  { type = ZTYPE_CMP_LS;  }
				else if (token == ">")  { type = ZTYPE_CMP_GT;  }
				else if (token == "++") { type = ZTYPE_INC;     }
				else if (token == "--") { type = ZTYPE_DEC;     }
				else if (token == "**") { type = ZTYPE_POW;     }
				else if (token == "<<") { type = ZTYPE_LSHT;    }
				else if (token == ">>") { type = ZTYPE_RSHT;    }
				else if (token == "=")  { type = ZTYPE_LASSIGN; }
				else if (token == "->") { type = ZTYPE_RASSIGN; }
				else { _D(LOG_CRIT, "Unknown token %s", token.c_str());	}
				goto END_LEXER;
				break;
			case '=':
				type      = ZTYPE_LASSIGN;
				token     = _linebuf_.substr(0, 1);
				_linebuf_ = _linebuf_.substr(1);
				goto END_LEXER;
				break;
			case '&':
				type      = ZTYPE_BIT_AND;
				token     = _linebuf_.substr(0, 1);
				_linebuf_ = _linebuf_.substr(1);
				goto END_LEXER;
				break;
			case '|':
				type      = ZTYPE_BIT_OR;
				token     = _linebuf_.substr(0, 1);
				_linebuf_ = _linebuf_.substr(1);
				goto END_LEXER;
				break;
			case '^':
				type      = ZTYPE_BIT_XOR;
				token     = _linebuf_.substr(0, 1);
				_linebuf_ = _linebuf_.substr(1);
				goto END_LEXER;
				break;
			case '.':									/* OPERATOR with single */
				type      = ZTYPE_DOT;
				token     = _linebuf_.substr(0, 1);
				_linebuf_ = _linebuf_.substr(1);
				goto END_LEXER;
				break;
			case ',':
				type      = ZTYPE_COMMA;
				token     = _linebuf_.substr(0, 1);
				_linebuf_ = _linebuf_.substr(1);
				goto END_LEXER;
				break;
			case ':':
				type      = ZTYPE_COLON;
				token     = _linebuf_.substr(0, 1);
				_linebuf_ = _linebuf_.substr(1);
				goto END_LEXER;
				break;
			case '(': case ')':							/* OPERATOR with pair */
				type = '(' == _linebuf_[0] ? ZTYPE_PAIR_GROUP_OPEN : ZTYPE_PAIR_GROUP_CLOSE;
				token = _linebuf_.substr(0, 1);
				_linebuf_ = _linebuf_.substr(1);
				goto END_LEXER;
				break;
			case '[': case ']':
				type = '[' == _linebuf_[0] ? ZTYPE_PAIR_BRAKES_OPEN: ZTYPE_PAIR_BRAKES_CLOSE;
				token = _linebuf_.substr(0, 1);
				_linebuf_ = _linebuf_.substr(1);
				goto END_LEXER;
				break;
			case '{': case '}':
				type = '{' == _linebuf_[0] ? ZTYPE_PAIR_DICT_OPEN : ZTYPE_PAIR_DICT_CLOSE;
				token = _linebuf_.substr(0, 1);
				_linebuf_ = _linebuf_.substr(1);
				goto END_LEXER;
				break;
			case '\"': case '\'':						/* string */
				pos = 1;
				while (_linebuf_[pos] != _linebuf_[0]) {
					if (_linebuf_[pos] == '\0') {
						std::string line;

						if (!std::getline(this->_fp_, line)) {
							_D(LOG_CRIT, "syntax error on #%-4d %s ", _lineno_,
																		_linebuf_.c_str());
						}
						_linebuf_ = _linebuf_ + line;
						_lineno_ ++;
					}
					pos ++;
				}
				type      = ZTYPE_STRING;
				token     = _linebuf_.substr(0, pos+1);
				_linebuf_ = _linebuf_.substr(pos+1);
				goto END_LEXER;
				break;
			default:
				while (ISWORD(_linebuf_[pos])) { pos ++; }
				type      = ZTYPE_IDENTIFIER;
				token     = _linebuf_.substr(0, pos);
				_linebuf_ = pos >= _linebuf_.size() ? "" : _linebuf_.substr(pos);

				     if (token == "include")		{ type = ZTYPE_CMD_INCLUDE;   }
				else if (token == "print")			{ type = ZTYPE_CMD_PRINT;     }
				else if (token == "func")			{ type = ZTYPE_CMD_FUNCTION;  }
				else if (token == "cls")			{ type = ZTYPE_CMD_CLASS;     }
				else if (token == "while")			{ type = ZTYPE_CMD_WHILE;     }
				else if (token == "for")			{ type = ZTYPE_CMD_FOR;       }
				else if (token == "in")				{ type = ZTYPE_CMD_IN;        }
				else if (token == "if")				{ type = ZTYPE_CMD_IF;        }
				else if (token == "elif")			{ type = ZTYPE_CMD_ELIF;      }
				else if (token == "else")			{ type = ZTYPE_CMD_ELSE;      }
				else if (token == "nop")			{ type = ZTYPE_CMD_NOP;       }
				else if (token == "continue")		{ type = ZTYPE_CMD_CONTINUE;  }
				else if (token == "break")			{ type = ZTYPE_CMD_BREAK;     }
				else if (token == "asm")			{ type = ZTYPE_CMD_ASM;       }
				else if (token == "and")			{ type = ZTYPE_LOG_AND;       }
				else if (token == "xor")			{ type = ZTYPE_LOG_XOR;       }
				else if (token == "or")				{ type = ZTYPE_LOG_OR;        }
				else if (token == "True")			{ type = ZTYPE_TRUE;          }
				else if (token == "False")			{ type = ZTYPE_FALSE;         }
				else if (token == "None")			{ type = ZTYPE_NONE;          }
				else if (token == "not")			{ type = ZTYPE_LOG_NOT;       }
				else if (token == "eq")				{ type = ZTYPE_CMP_EQ;        }
				else if (token == "return")			{ type = ZTYPE_CMD_RETURN;    }

				else if (token == "exit")			{ type = ZTYPE_BUILTIN_EXIT;   }
				else if (token == "syscall")		{ type = ZTYPE_BUILTIN_SYSCALL;}

				goto END_LEXER;
				break;
		}
	} while (1);
INDENT_PROCESSOR:
	_D(LOG_DEBUG_LEXER, "current indent status %d / %d", _indent_cur_, _indent_cnt_);
	if (_indent_cur_ < _indent_cnt_) {
		_indent_cnt_ --;
		type  = ZTYPE_DEDENT;
		token = "[DEDENT]";
		goto END_LEXER;
	} else if (_indent_cur_ > _indent_cnt_) {
		_indent_cnt_ ++;
		type  = ZTYPE_INDENT;
		token = "[INDENT]";
		goto END_LEXER;
	}
END_LEXER:
	if (type == ZTYPE_NEWLINE && blNewline && !blEOF) goto LEXER;

	blNewline = type == ZTYPE_NEWLINE;
	_D(LOG_DEBUG_LEXER, "token -> #0x%-4X %s", type, token.c_str());
	return std::make_pair(token, type);
}
