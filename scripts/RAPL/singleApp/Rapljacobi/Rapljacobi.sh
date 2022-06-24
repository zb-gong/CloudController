mkdir -p results

export HEARTBEAT_ENABLED_DIR=heartbeat
export OMP_NUM_THREADS=8

rm -Rf ${HEARTBEAT_ENABLED_DIR}

SET_SPEED=/home/zibo/Projects/PUPIL/tools/pySetCPUSpeed.py
RAPL_POWER_MON = /home/zibo/Projects/PUPIL/src/RAPL/RaplPowerMonitor

#sudo ls > /dev/null

mkdir -p ${HEARTBEAT_ENABLED_DIR}
$SET_SPEED -S 0
for i in 1 2 3 4; do

    sudo $RAPL_POWER_MON &
    HR=''
    power=''
    joules=''
    FinalHR=''

    m=$(expr 10 + $i \* 5)


    sudo /home/zibo/Projects/PUPIL/src/RAPL/RaplSetPower $m
    sudo -E numactl --interleave=0-0 --physcpubind=0-7 /home/zibo/Projects/PUPIL/patches/jacobi/jacobi
    sudo pkill RaplPowerMonitor

    HR=$(tail -n 1 jacobi_heartbeat.log | awk '// {print $4}')
    power=$(cat power.txt | awk '/Pavg/ {print $2}')
    FinalHR=$(cat jacobi_heartbeat.log | awk '{sum+=$5} END {print sum/NR}')

    echo $m $HR $FinalHR $power >>Rapljacobi.results
    sleep 5
done

MEMS=$(ipcs | grep root | awk '{print $2}')
for k in $MEMS; do
    echo sudo Freeing $k
    sudo ipcrm -m $k
done

MEMS=`ipcs | grep huazhe | awk '{print $2}'`
for k in $MEMS; do echo Freeing $k; ipcrm -m $k; done
