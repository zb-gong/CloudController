export OMP_NUM_THREADS=8
sudo truncate -s 0 *.log
SET_SPEED=/home/zibo/Projects/PUPIL/tools/pySetCPUSpeed.py
RAPL_POWER_MON=/home/zibo/Projects/PUPIL/src/RAPL/RaplPowerMonitor

sudo -E $SET_SPEED -S 0
m=$1

sudo $RAPL_POWER_MON &

sudo /home/zibo/Projects/PUPIL/src/RAPL/RaplSetPower $m
sudo -E numactl --interleave=0-0 --physcpubind=0-7 /home/zibo/Projects/PUPIL/patches/jacobi/jacobi
sudo pkill RaplPowerMonitor

sleep 5
