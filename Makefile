include Makefile.in

SUBDIR=src

VIMRC=$(wildcard *.vim)
BIN=zasm


.PHONY: all clean install $(SUBDIR)

all: $(BIN)

$(BIN): $(SUBDIR)
	ln -sf src/$@.inst $@


$(SUBDIR):
	ln -sf ../Makefile.in $@/Makefile.in
	$(MAKE) -C $@ $(MAKECMDGOALS)

clean: $(SUBDIR)

install: src
	install -d ~/.vim/syntax/
	install -m644 $(VIMRC) ~/.vim/syntax/
	install -m755 src/zasm.inst /usr/local/bin/zasm

