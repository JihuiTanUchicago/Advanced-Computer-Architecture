CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
TARGET = cache-sim

SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

DEPS = Address.hpp DataBlock.hpp Ram.hpp Cache.hpp Cpu.hpp

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $<

.PHONY: clean

clean:
	rm -f $(TARGET) *.o