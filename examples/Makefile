SRC=$(wildcard *.zs)
CASES=$(subst .zs,,$(SRC))

.PHONY: $(CASES)

helloWorld: helloWorld.zs
	../zasm -o $@ $^
	test "`./$@ 2>&1 >/dev/null`" == 'Welcome to ZASM - A prototype for a simple assembler'
	test "`./$@ 2>/dev/null`"     == 'Hello World'

.PHONY: clean

clean:
	rm -f $(CASES)
