for (( ; ;)) do 
  sudo docker stats --no-stream 96edd256c25b | tail -1 | awk '{print $3}' >> docker_util.txt
  echo ----------------- 
  sleep 2
done