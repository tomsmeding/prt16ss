uname_str := $(shell uname)
ifeq ($(uname_str),Darwin)
	CXX = g++-g
else
	CXX = /vol/share/software/gcc/5.3.0/bin/g++ -Wl,--rpath=/vol/share/software/gcc/5.3.0/lib64
endif
CXXFLAGS = -Wall -Wextra -std=c++14 -O2
LDFLAGS = -lncurses
BIN = main

obj_files = $(patsubst %.cpp,%.o,$(wildcard *.cpp))


.PHONY: all clean remake test switch

all: $(BIN)

clean:
	rm -f $(BIN) *.o

remake: clean all

test: all
	python3 modeltest.py

switch:
	test -e main.cpp && mv main.cpp main.cpp_ || mv main.cpp_ main.cpp
	test -e modeltestmain.cpp && mv modeltestmain.cpp modeltestmain.cpp_ || mv modeltestmain.cpp_ modeltestmain.cpp


$(BIN): $(obj_files)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp *.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<
