POWER_CAP=$1
sudo truncate -s 0 *.log
sudo -E python3 power-control-h.py $POWER_CAP jacobi asf sudo -E ../../patches/jacobi/jacobi
# sudo -E python3 power-control-h.py $POWER_CAP STREAM asd sudo -E ../../patches/STREAM/stream_c.exe
