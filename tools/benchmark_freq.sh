for i in {1600..3700..100} 
do
  echo "-------------start freq: $i----------------"
  sudo ./setspeed -a -f $i
  sudo docker rm /client
  sudo docker run -it --name client --net search_network cloudsuite3/web-search:client 172.18.0.2 100 90 60 60 >> results.txt
  echo "--------------freq:$i finished--------------"
  sleep 2
done