EXECUTABLE := virt
CC = g++
CFLAGS = -Wall -O3
LDFLAGS = $(shell pkg-config --libs libvirt)

.PHONY: clean

all: $(EXECUTABLE)

$(EXECUTABLE): virt.cpp
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(EXECUTABLE)