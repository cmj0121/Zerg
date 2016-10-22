include Makefile.in

ZASM_SRC=$(wildcard zasm/*.cpp)
ZASM_OBJ=$(subst .cpp,.o,$(ZASM_SRC))

ZERG_SRC=$(wildcard zerg/*.cpp)
ZERG_OBJ=$(subst .cpp,.o,$(ZERG_SRC))

GRAMMAR=Grammar/Grammar

BIN=zasm.inst zerg.inst

CXXFLAGS:=$(CXXFLAGS) -I../include

.PHONY: all clean install

all: $(BIN)

zasm.inst: zasm.cpp $(ZASM_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^
	$(STRIP) $@

zerg.inst: zerg.cpp $(ZASM_OBJ) $(ZERG_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ zerg.cpp $(ZASM_OBJ) $(ZERG_OBJ)
	$(STRIP) $@

clean:
	rm -f $(BIN) $(ZASM_OBJ) $(ZERG_OBJ)

install: $(BIN)
	install -m755 zasm.inst /usr/local/bin/zasm
	install -m755 zerg.inst /usr/local/bin/zerg

