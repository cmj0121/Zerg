include Makefile.in

SUBDIR=src

VIMRC=$(wildcard *.vim)
BIN=zasm zgir zerg


.PHONY: all clean install install-syntax $(SUBDIR) examples

all: $(BIN) examples CI

$(BIN): $(SUBDIR)
	$(DEBUG) ln -sf src/$@.inst $@

examples: $(BIN) install

$(SUBDIR):
	$(DEBUG) ln -sf ../Makefile.in $@/Makefile.in
	$(MAKE) -C $@ $(MAKECMDGOALS)

clean: $(SUBDIR)
	$(MAKE) -C examples $(MAKECMDGOALS)
	rm -f $(BIN)

install: $(SUBDIR) install-syntax
	install -m755 -d ~/.vim/syntax/
	install -m775 -d /usr/local/lib/zerg/
	install -m644 LIBS/* /usr/local/lib/zerg/

install-syntax: $(VIMRC)
	install -m644 $(VIMRC) ~/.vim/syntax/

grammar: $(SUBDIR)

.PHONY: CI

CI:
	install -m755 CI/* .git/hooks/
