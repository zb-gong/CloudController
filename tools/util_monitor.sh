for (( ; ;)) do 
  pgrep python3 | xargs -I {} ps -p {} -L -o psr,pcpu
  echo ----------------- 
  sleep 1
done