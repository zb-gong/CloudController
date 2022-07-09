for (( ; ; ))
do
  cpupower -c 0 frequency-info | grep "current CPU" | awk 'NR>1 {print}'
done