include Makefile.in

ZASM_SRC=$(wildcard zasm/*.cpp)
ZASM_OBJ=$(subst .cpp,.o,$(ZASM_SRC))

ZGIR_SRC=$(wildcard ir/*.cpp)
ZGIR_OBJ=$(subst .cpp,.o,$(ZGIR_SRC))

ZERG_SRC=$(wildcard zerg/*.cpp)
ZERG_OBJ=$(subst .cpp,.o,$(ZERG_SRC))

CASES_SRC=$(wildcard testcases/*.cpp)
CASES_OBJ=$(subst .cpp,.o,$(CASES_SRC))
CASES=$(subst .cpp,.testcase,$(CASES_SRC))

BIN=zasm.inst zgir.inst zerg.inst

CXXFLAGS:=$(CXXFLAGS) -I../include

.PHONY: all clean install testcases

all: $(BIN) $(CASES)

zasm/macho64.o: zasm/macho64_inc.hpp

zasm.inst: $(ZASM_OBJ)
zgir.inst: $(ZGIR_OBJ) $(ZASM_OBJ)
zerg.inst: $(ZERG_OBJ) $(ZGIR_OBJ) $(ZASM_OBJ)

%.inst: %.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^
	$(DEBUG) $(STRIP) $@

clean:
	rm -f $(BIN) $(ZASM_OBJ) $(ZERG_OBJ)

install: $(BIN)
	install -m755 zasm.inst /usr/local/bin/zasm
	install -m755 zerg.inst /usr/local/bin/zerg
$(CASES): $(ZASM_OBJ) $(ZGIR_OBJ) $(ZERG_OBJ)

%.testcase: %.o
	$(DEBUG) $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^
	$(DEBUG) ./$@

