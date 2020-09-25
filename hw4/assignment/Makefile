CXX = g++
CXXFLAGS = -std=c++11 -Wall -O3 -mcpu=native -fno-tree-vectorize -DNDEBUG -pthread
LDFLAGS = -lpthread -pthread
INCLUDES = -I./common

EXECUTABLES = baseline neon_filter example
SRC = $(wildcard src/*.cpp common/*.cpp)
EXAMPLE_SRC = $(wildcard neon_example/*.cpp common/*.cpp)

.PHONY: clean

all: $(EXECUTABLES)
	@echo $(EXECUTABLES) compiled!

baseline: $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

neon_filter: $(SRC)
	$(CXX) -DVECTORIZED $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

example: $(EXAMPLE_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

clean:
	$(RM) -rf perf* *.bin *.txt *.dSYM *.out *.o *~ $(EXECUTABLES)
