ZASM_SRC=$(wildcard *.zs)
ZERG_SRC=$(wildcard *.zg)
CASES=$(subst .zs,.case,$(ZASM_SRC)) $(subst .zg,.case,$(ZERG_SRC))

.PHONY: all

all: $(CASES)

helloWorld.case: helloWorld.zs
	@../zasm -o $@ $^
	test "`./$@ 2>&1 >/dev/null`" == 'Welcome to ZASM - A prototype for a simple assembler'
	test "`./$@ 2>/dev/null`"     == 'Hello World'
	@../zasm --symbol -o $@ $^
	test "`./$@ 2>&1 >/dev/null`" == 'Welcome to ZASM - A prototype for a simple assembler'
	test "`./$@ 2>/dev/null`"     == 'Hello World'

arithmetic.case: arithmetic.zg
	@../zerg -o $@ $^
	test `./$@`                   == 7
	@../zerg -r $^ > $^r
	@../zerg -o $@ -R $^r
	test `./$@`                   == 7

exit.case: exit.zg
	@../zerg -o $@ $^
	@./$@ || test "$$?"            == 4
	@../zerg -r $^ > $^r
	@../zerg -o $@ -R $^r
	@./$@ || test "$$?"            == 4

condition.case: condition.zg
	@../zerg -o $@ $^
	test `./$@ | grep 1 | wc -l` == 2
	@../zerg -r $^ > $^r
	@../zerg -o $@ -R $^r
	test `./$@ | grep 1 | wc -l` == 2

repeat.case: repeat.zg
	@../zerg -o $@ $^
	test `./$@    | grep 1 | wc -l` == 5
	test `./$@    | grep 0 | wc -l` == 1
	@../zerg -r $^ > $^r
	@../zerg -o $@ -R $^r
	test `./$@    | grep 1 | wc -l` == 5
	test `./$@    | grep 0 | wc -l` == 1

buffer.case: buffer.zg
	@../zerg -o $@ $^
	test `./$@`                       == "cmj"
	@../zerg -r $^ > $^r
	@../zerg -o $@ -R $^r
	test `./$@`                       == "cmj"

.PHONY: clean

clean:
	rm -f $(CASES)
