EXEC := controller
M_EXEC := monitor
OBJDIR = build
LOGDIR = log
CXX = g++
CXXFLAGS = -Wall -O3 -g -std=c++11 $(shell pkg-config --cflags raplcap-msr) $(shell pkg-config --cflags libvirt)-fopenmp -lm
LDFLAGS = $(shell pkg-config --libs --static raplcap-msr) $(shell pkg-config --libs libvirt)

C_OBJS = $(OBJDIR)/cmain.o
M_OBJS = $(OBJDIR)/mmain.o
OBJS = $(OBJDIR)/util.o $(OBJDIR)/controller.o $(OBJDIR)/monitor.o
 
.PHONY: clean

all: $(EXEC) $(M_EXEC)

dirs:
	mkdir -p $(OBJDIR)/
	mkdir -p $(LOGDIR)/

$(EXEC): dirs $(OBJS) $(C_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(C_OBJS) $(LDFLAGS)

$(M_EXEC): dirs $(OBJS) $(M_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(M_OBJS) $(LDFLAGS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(EXEC) $(OBJDIR)/* $(M_EXEC) $(LOGDIR)/*