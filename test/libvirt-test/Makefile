EXECUTABLE := virt
CC = g++
CFLAGS = -Wall -O3 -std=c++17 $(shell pkg-config --cflags raplcap-msr) -fopenmp -lpthread
LDFLAGS = $(shell pkg-config --libs libvirt) $(shell pkg-config --libs --static raplcap-msr)

.PHONY: clean

all: $(EXECUTABLE)

$(EXECUTABLE): virt.cpp
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(EXECUTABLE)