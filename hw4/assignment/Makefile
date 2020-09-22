CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2 -g -DNDEBUG -pthread
LDFLAGS = -lpthread -pthread
INCLUDES = -I./common

EXECUTABLES = base walkthrough
BASELINE_SRC = $(wildcard baseline/*.cpp common/*.cpp)
WALKTHROUGH_SRC = Walkthrough.cpp $(wildcard common/*.cpp)

.PHONY: clean

all: $(EXECUTABLES)
	@echo $(EXECUTABLES) compiled!

base: $(BASELINE_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

walkthrough: $(WALKTHROUGH_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

clean:
	$(RM) -rf perf* *.bin *.txt *.dSYM *.out *.o *~ $(EXECUTABLES)
