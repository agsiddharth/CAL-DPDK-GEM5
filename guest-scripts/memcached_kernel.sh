ip link set dev eth0 up
ifconfig eth0 10.10.10.10
sleep 3; m5 checkpoint;

echo "Starting memcached server in Kernel mode"
memcached -p 0 -U 11211 -u root -vvv
