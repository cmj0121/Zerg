include Makefile.in

SUBDIR=src

VIMRC=$(wildcard *.vim)
BIN=zasm zerg


.PHONY: all clean install $(SUBDIR) examples

all: $(BIN) examples

$(BIN): $(SUBDIR)
	ln -sf src/$@.inst $@

examples: $(BIN)

$(SUBDIR):
	ln -sf ../Makefile.in $@/Makefile.in
	$(MAKE) -C $@ $(MAKECMDGOALS)

clean: $(SUBDIR)
	rm -f $(BIN)

install: $(SUBDIR)
	install -d ~/.vim/syntax/
	install -m644 $(VIMRC) ~/.vim/syntax/

grammar: $(SUBDIR)
