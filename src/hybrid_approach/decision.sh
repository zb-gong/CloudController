POWER_CAP=$1
sudo truncate -s 0 *.log
sudo -E python3 power-control-hr-pstree.py $POWER_CAP jacobi sudo -E ../../patches/jacobi/jacobi
# sudo -E python3 power-control-hr-pstree.py $POWER_CAP STREAM sudo -E ../../patches/STREAM/stream_c.exe
