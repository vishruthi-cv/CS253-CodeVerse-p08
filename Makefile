# Makefile -- one command builds everything:   make
#             run the tests:                   make test
#             rerun every benchmark and figure: make bench   (needs bash and Python 3)
#             delete the build folder:         make clean

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2
INC      := -Iinclude -Isrc/structA/include -Isrc/structB/include
SRC      := $(wildcard src/structA/src/*.cpp) $(wildcard src/structB/src/*.cpp)

# On Windows (MSYS2 / Git Bash) the memory code in bench needs psapi, and a static link
# avoids a runtime mismatch crash we saw there.
ifneq (,$(findstring MINGW,$(shell uname -s)))
LIBS := -static -lpsapi
endif
ifneq (,$(findstring MSYS,$(shell uname -s)))
LIBS := -static -lpsapi
endif

all: build/app build/bench build/oracle_diff build/adversary

build:
	mkdir -p build

build/app: app/src/main.cpp $(SRC) | build
	$(CXX) $(CXXFLAGS) $(INC) app/src/main.cpp $(SRC) -o $@

build/bench: bench/bench.cpp $(SRC) | build
	$(CXX) $(CXXFLAGS) $(INC) bench/bench.cpp $(SRC) -o $@ $(LIBS)

build/oracle_diff: oracle/oracle_diff.cpp $(SRC) | build
	$(CXX) $(CXXFLAGS) $(INC) oracle/oracle_diff.cpp $(SRC) -o $@

build/adversary: adversary/adversary.cpp | build
	$(CXX) $(CXXFLAGS) adversary/adversary.cpp -o $@

# The differential test against the oracle runs last, so a passing run ends with ALL TESTS PASSED.
test: build/bench build/oracle_diff
	./build/bench check 1000 100000 10 50 200000 1
	./build/bench check 1000 100000 10 90 200000 2
	./build/bench check 50 100 30 50 100000 3
	./build/oracle_diff

bench:
	bash scripts/run_all.sh

clean:
	rm -rf build

.PHONY: all test bench clean
