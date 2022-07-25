EXECUTABLE := rapl-powercap
CC = gcc
CFLAGS = -Wall -O3 $(shell pkg-config --cflags raplcap-msr)
LDFLAGS = $(shell pkg-config --libs --static raplcap-msr)

.PHONY: clean

all: $(EXECUTABLE)

$(EXECUTABLE): rapl-powercap.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(EXECUTABLE)