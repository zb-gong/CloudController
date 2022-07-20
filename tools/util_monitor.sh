for (( ; ;)) do 
  # echo 815086 | xargs -I {} ps -p {} -L -o psr,pcpu > util.txt
  # echo 707254 | xargs -I {} ps -p {}  -o pcpu
  top -b -n 2 -d 0.2 -p 781380 | tail -1 | awk '{print $9}' >> top_util.txt
  echo ----------------- 
  sleep 2
done