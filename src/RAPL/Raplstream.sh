# export OMP_NUM_THREADS=4
sudo truncate -s 0 *.log
SET_SPEED=/home/zibo/Projects/PUPIL/tools/pySetCPUSpeed.py
RAPL_POWER_MON=/home/zibo/Projects/PUPIL/src/RAPL/RaplPowerMonitor

m=$1
core=$(($2-1))

sudo $RAPL_POWER_MON &

# sudo /home/zibo/Projects/PUPIL/src/RAPL/RaplSetPower $m
sudo -E $SET_SPEED -S 11
sudo -E numactl --interleave=0-0 --physcpubind=0-$core /home/zibo/Projects/PUPIL/patches/STREAM/stream_c.exe
# sudo -E /home/zibo/Projects/PUPIL/patches/jacobi/jacobi
sudo pkill RaplPowerMonitor

sleep 5
python3 perf-analysis.py
