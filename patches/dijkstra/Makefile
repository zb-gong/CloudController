

# LDFLAGS=-L/home/hankhoffmann/heartbeats-development/heartbeats-google-code/heartbeats/lib
LDFLAGS=-L/home/zibo/Projects/PUPIL/lib/heartbeats/lib
CC=gcc
# CFLAGS := -I/home/hankhoffmann/heartbeats-development/heartbeats-google-code/heartbeats/inc/ -O3 -Wall -MD -fopenmp # -g -fno-omit-frame-pointer
CFLAGS := -I/home/zibo/Projects/PUPIL/lib/heartbeats/inc/ -no-pie -O3 -Wall -MD -fopenmp # -g -fno-omit-frame-pointer

OBJS := dijkstra-pt-b.o
LIBS=-lhb-shared -lhb-acc-pow-shared

all: dijkstra-pt-hb-dvfs


dijkstra-pt-hb: $(OBJS) weights3.h
	gcc -o $@ $(LDFLAGS) $(CFLAGS) $^ -lpthread -lhb-shared -lrt -lhb-acc-pow-shared

dijkstra-pt-hb2:  dijkstra-pt-hb-dvfs.c weights3.h
	gcc -o $@ $(LDFLAGS) $(CFLAGS) $^ -lpthread -lrt -lhb-acc-pow-shared -lrt -lm



