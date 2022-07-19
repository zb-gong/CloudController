for (( ; ; ))
do
  cpupower -c 0 frequency-info | grep "current CPU" | awk 'NR>1 {print}' >> runtime_freq.txt
  # sudo docker stats --no-stream 96edd256c25b | tail -1 | awk '{print $3}' >> runtime_freq.txt
  top -b -n 2 -d 0.2 -p 815086 | tail -1 | awk '{print $9}' >> runtime_freq.txt
  echo -------------------------------------------------- >> runtime_freq.txt
  sleep 1
done