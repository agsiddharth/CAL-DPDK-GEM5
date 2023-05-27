ip link set dev eth0 up
ifconfig eth0 10.212.84.119 netmask 255.255.255.0
ifconfig
sleep 1; m5 checkpoint;

echo "Starting memcached server in Kernel mode"
memcached -p 0 -U 11211 -u root
# memcached -p 0 -U 11211 -u root -vvv
# tcpdump -i eth0 -X
