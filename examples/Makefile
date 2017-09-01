ZASM_SRC=$(wildcard *.zs)
ZERG_SRC=$(wildcard *.zg)
ZGIR_SRC=$(wildcard *.zgr)

OS=$(shell uname)
ifeq ($(OS), Darwin)
ZASM_CASES=$(subst .zs,.case,$(ZASM_SRC))
ZERG_CASES=$(subst .zg,.case,$(ZERG_SRC))
ZGIR_CASES=$(subst .zgr,.case,$(ZGIR_SRC))
else
ZASM_CASES=
ZERG_CASES=
endif

.PHONY: all zerg zasm zgir

all: zerg zasm zgir

zasm: $(ZASM_CASES)
zerg: $(ZERG_CASES)
zgir: $(ZGIR_CASES)

helloWorld.case: helloWorld.zs
	@../zasm -o $@ $^
	test "`./$@ 2>&1 >/dev/null`" == 'Welcome to ZASM - A prototype for a simple assembler'
	test "`./$@ 2>/dev/null`"     == 'Hello World - Zasm'
	@../zasm --symbol -o $@ $^
	test "`./$@ 2>&1 >/dev/null`" == 'Welcome to ZASM - A prototype for a simple assembler'
	test "`./$@ 2>/dev/null`"     == 'Hello World - Zasm'

simpleos.case: simpleos.zs
	@../zasm -fbin -o $@ $^

arithmetic.case: arithmetic.zg
	@../zerg -o $@ $^
	test `./$@`                   == 7

exit.case: exit.zg
	@../zerg -o $@ $^
	@./$@ || test "$$?"            == 4

function.case: function.zg
	@../zerg -o $@ $^
	@./$@ || test "$$?"            == 4

condition.case: condition.zg
	@../zerg -o $@ $^
	test `./$@ | grep 1 | wc -l` == 2

repeat.case: repeat.zg
	@../zerg -o $@ $^
	test `./$@    | grep 1 | wc -l` == 5
	test `./$@    | grep 0 | wc -l` == 1

buffer.case: buffer.zg
	@../zerg -o $@ $^
	test `./$@`                       == "cmj"
	test `./$@`                       == "cmj"

simple_ir.case: simple_ir.zgr
	@../zgir -o $@ $^
	@./$@ || test "$$?"            == 4
	test "`./$@ 2>/dev/null`"     == 'Simple Zerg IR program'
	test "`./$@ 2>&1 >/dev/null`" == 'string created from buffer'

.PHONY: clean

clean:
	rm -f *.case
