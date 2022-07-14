CC=gcc
CFLAGS= -Wall
LIBS=-lm
SRCS= RaplPowerLimitDisable.c RaplSetPowerSeprate.c RaplPowerMonitor.c RaplSetPower.c RaplPowerCheck.c
#OBJS= $(SRCS:.c=)
all: RaplPowerLimitDisable RaplSetPowerSeprate RaplPowerMonitor RaplSetPower RaplPowerCheck

RaplPowerLimitDisable: RaplPowerLimitDisable.c
	$(CC) $(CFLAGS) -o RaplPowerLimitDisable RaplPowerLimitDisable.c $(LIBS)
RaplSetPowerSeprate: RaplSetPowerSeprate.c
	$(CC) $(CFLAGS) -o RaplSetPowerSeprate RaplSetPowerSeprate.c $(LIBS)
RaplSetPower: RaplSetPower.c
	$(CC) $(CFLAGS) -o RaplSetPower RaplSetPower.c $(LIBS)
RaplPowerMonitor: RaplPowerMonitor.c
	$(CC) $(CFLAGS) -o RaplPowerMonitor RaplPowerMonitor.c $(LIBS)
RaplPowerCheck: RaplPowerCheck.c
	$(CC) $(CFLAGS) -o RaplPowerCheck RaplPowerCheck.c $(LIBS)
clean:
	rm -f RaplPowerMonitor RaplSetPowerSeprate RaplPowerLimitDisable RaplSetPower RaplPowerCheck
