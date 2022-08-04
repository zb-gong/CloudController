EXEC := controller
OBJDIR = build
CXX = g++
CXXFLAGS = -Wall -O3 -std=c++11 $(shell pkg-config --cflags raplcap-powercap) -fopenmp
LDFLAGS = $(shell pkg-config --libs --static raplcap-powercap)

OBJS = $(OBJDIR)/main.o $(OBJDIR)/util.o $(OBJDIR)/controller.o 
 
.PHONY: clean

all: $(EXEC)

dirs:
	mkdir -p $(OBJDIR)/

$(EXEC): dirs $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(EXEC)