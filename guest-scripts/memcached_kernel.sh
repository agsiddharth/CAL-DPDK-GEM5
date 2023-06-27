ip link set dev eth0 down
ifconfig eth0 192.17.100.7 netmask 255.255.252.0
ip link set dev eth0 up
arp -s 192.17.100.243 00:80:00:00:00:01​
echo "Starting memcached server in Kernel mode"
memcached -p 0 -U 11211 -u root
# memcached -p 0 -U 11211 -u root -vvv
# tcpdump -i eth0 -X
