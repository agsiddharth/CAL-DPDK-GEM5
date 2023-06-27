ip link set dev eth0 down
arp -s 192.17.100.243 00:EB:24:B2:05:AC
ifconfig eth0 192.17.100.7 netmask 255.255.252.0
ip link set dev eth0 up
echo "Starting memcached server in Kernel mode"
memcached -p 0 -U 11211 -vv -u root
# memcached -p 0 -U 11211 -u root -vvv
# tcpdump -i eth0 -X
