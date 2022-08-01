for (( ; ; ))
do
  sudo perf stat -e cache-misses,cache-references,cycles,instructions -p 815086 sleep 2
done
# sudo perf record -p 815086 sleep 2
# sudo perf report
# sudo perf mem report