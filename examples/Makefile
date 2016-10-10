ZASM_SRC=$(wildcard *.zs)
ZERG_SRC=$(wildcard *.zg)
CASES=$(subst .zs,.case,$(ZASM_SRC)) $(subst .zg,.case,$(ZERG_SRC))

.PHONY: all

all: $(CASES)

helloWorld.case: helloWorld.zs
	../zasm -o $@ $^
	test "`./$@ 2>&1 >/dev/null`" == 'Welcome to ZASM - A prototype for a simple assembler'
	test "`./$@ 2>/dev/null`"     == 'Hello World'

exit.case: exit.zg
	../zerg -o $@ $^


.PHONY: clean

clean:
	rm -f $(CASES)
