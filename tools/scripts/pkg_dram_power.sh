for (( ; ; ))
do
  sudo turbostat --quiet --cpu 0-9 --show RAMWatt --interval 1 --num_iteration 1 | awk 'NR==2 {print}' >> ../ramwatt.txt
  # echo --------------------------------------------------
  # sleep 1
done