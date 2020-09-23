CXX = g++
CXXFLAGS = -std=c++11 -Wall -O3 -DNDEBUG -pthread
LDFLAGS = -lpthread -pthread
INCLUDES = -I./common

EXECUTABLES = baseline vectorized
SRC = $(wildcard src/*.cpp common/*.cpp)

.PHONY: clean

all: $(EXECUTABLES)
	@echo $(EXECUTABLES) compiled!

baseline: $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

vectorized: $(SRC)
	$(CXX) -DVECTORIZED $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

clean:
	$(RM) -rf perf* *.bin *.txt *.dSYM *.out *.o *~ $(EXECUTABLES)
