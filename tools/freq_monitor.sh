for (( ; ; ))
do
  cpupower -c 0 frequency-info | grep "current CPU" | awk 'NR>1 {print}' >> runtime_freq.txt
  cpupower -c 1 frequency-info | grep "current CPU" | awk 'NR>1 {print}' >> runtime_freq.txt
  echo -------------------------------------------------- >> runtime_freq.txt
  sleep 1
done