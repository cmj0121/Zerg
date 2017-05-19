include Makefile.in

SUBDIR=src

VIMRC=$(wildcard *.vim)
BIN=zasm zerg


.PHONY: all clean install $(SUBDIR) examples

all: $(BIN) examples CI

$(BIN): $(SUBDIR)
	ln -sf src/$@.inst $@

examples: $(BIN) install

$(SUBDIR):
	ln -sf ../Makefile.in $@/Makefile.in
	$(MAKE) -C $@ $(MAKECMDGOALS)

clean: $(SUBDIR)
	$(MAKE) -C examples $(MAKECMDGOALS)
	rm -f $(BIN)

install: $(SUBDIR)
	install -d ~/.vim/syntax/
	install -m644 $(VIMRC) ~/.vim/syntax/
	install -d /usr/local/lib/zerg/
	install -m644 LIBS/* /usr/local/lib/zerg/

grammar: $(SUBDIR)

.PHONY: CI

CI:
	install -m755 CI/* .git/hooks/
