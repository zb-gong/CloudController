for i in {0..2..1} 
do
  echo "-------------start core: $i----------------"
  sudo docker update --cpuset-cpus 0-$i 96edd256c25b
  sudo docker rm /client
  sudo docker run -it --name client --net search_network cloudsuite3/web-search:client 172.18.0.2 100 90 60 60 >> results.txt
  echo "--------------core:$i finished--------------"
  sleep 2
done