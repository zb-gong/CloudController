for (( ; ;))
do
  # sudo /home/cc/CloudController/src/RAPL/RaplPowerCheck >> power.txt
  sudo turbostat --quiet --cpu 0-7 --show PkgWatt --interval 2 --num_iteration 1 | awk "NR==2{print}" >> power.txt
  echo ----------------- 
  sleep 2
done