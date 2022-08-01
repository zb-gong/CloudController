for i in {50..500..50} 
do
  echo "-------------start user: $i----------------"
  # sudo /home/cc/PUPIL/src/RAPL/RaplSetPowerSeprate $i $i
  for j in {60..100..10}
  do
    echo "-------------start powercap: $j----------------"
    python3 /home/cc/PUPIL/src/controller/naive-freq-core.py $j 8 96edd256c25b &
    sudo docker rm /client
    sudo docker run -it --name client --net search_network cloudsuite3/web-search:client 172.18.0.2 $i 60 60 60 >> results.txt
    sleep 3
  done
  echo "--------------user:$i finished--------------"
done