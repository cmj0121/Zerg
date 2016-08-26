include Makefile.in

ZASM_SRC=$(wildcard zasm/*.cpp)
ZASM_OBJ=$(subst .cpp,.o,$(ZASM_SRC))

BIN=zasm.inst

CXXFLAGS:=$(CXXFLAGS) -I../include

.PHONY: all clean install

all: $(BIN)

zasm.inst: $(ZASM_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^
	$(STRIP) $@

clean:
	rm -f $(BIN) $(ZASM_OBJ)

install:
