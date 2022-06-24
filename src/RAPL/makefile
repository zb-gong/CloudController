CC=gcc
CFLAGS= -Wall
LIBS=-lm
SRCS= RaplPowerLimitDisable.c RaplSetPowerSeprate.c RaplPowerMonitor.c RaplSetPower.c
#OBJS= $(SRCS:.c=)
all: RaplPowerLimitDisable RaplSetPowerSeprate RaplPowerMonitor RaplSetPower

RaplPowerLimitDisable: RaplPowerLimitDisable.c
	$(CC) $(CFLAGS) -o RaplPowerLimitDisable RaplPowerLimitDisable.c $(LIBS)
RaplSetPowerSeprate: RaplSetPowerSeprate.c
	$(CC) $(CFLAGS) -o RaplSetPowerSeprate RaplSetPowerSeprate.c $(LIBS)
RaplSetPower: RaplSetPower.c
	$(CC) $(CFLAGS) -o RaplSetPower RaplSetPower.c $(LIBS)
RaplPowerMonitor: RaplPowerMonitor.c
	$(CC) $(CFLAGS) -o RaplPowerMonitor RaplPowerMonitor.c $(LIBS)
clean:
	rm -f RaplPowerMonitor RaplSetPowerSeprate RaplPowerLimitDisable
