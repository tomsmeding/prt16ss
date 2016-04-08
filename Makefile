CXX = /vol/share/software/gcc/5.3.0/bin/g++ -Wl,--rpath=/vol/share/software/gcc/5.3.0/lib64
CXXFLAGS = -Wall -Wextra -std=c++14 -O2
LDFLAGS = -lncurses
BIN = main

obj_files = $(patsubst %.cpp,%.o,$(wildcard *.cpp))


.PHONY: all clean remake

all: $(BIN)

clean:
	rm -f $(BIN) *.o

remake: clean all


$(BIN): $(obj_files)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp *.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<
