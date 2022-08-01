all: setspeed resetspeed

setspeed: CSetCPUSpeed.c
	gcc -O3 -o $@ $< -fopenmp

resetspeed: CResetCPUSpeed.c
	gcc -O3 -o $@ $< -fopenmp

clean:
	rm setspeed resetspeed