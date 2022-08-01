for i in {30..50..5} 
do
  echo "-------------start power: $i*2----------------"
  sudo /home/cc/PUPIL/src/RAPL/RaplSetPowerSeprate $i $i
  sudo docker rm /client
  sudo docker run -it --name client --net search_network cloudsuite3/web-search:client 172.18.0.2 250 60 60 60 >> results.txt
  echo "--------------power:$i*2 finished--------------"
  sleep 10
done