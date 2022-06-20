EXE=jacobi

$(EXE): jacobi.c
	gcc -g -O3 -no-pie -L/home/zibo/Projects/PUPIL/lib/heartbeats/lib -I/home/zibo/Projects/PUPIL/lib/heartbeats/inc/ jacobi.c -o jacobi -lhb-acc-pow-shared -lm -lrt -fopenmp

clean:
	rm -rf jacobi jacobi_heartbeat.log
