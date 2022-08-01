EXEC := controller
CXX = g++
CXXFLAGS = -Wall -O3 -std=c++11 $(shell pkg-config --cflags raplcap-powercap)
LDFLAGS = $(shell pkg-config --libs --static raplcap-powercap)

.PHONY: clean

all: $(EXEC)

$(EXEC): main.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(EXEC)