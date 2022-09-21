# Retrieve dist-gem5 rank and size parameters using magic keys
MY_RANK=$(m5 initparam dist-rank)
MY_SIZE=$(m5 initparam dist-size)

/bin/hostname tux${MY_RANK}

MY_ADDR=$(($MY_RANK + 2))


ip link set eth0 address 00:90:00:00:00:0${MY_ADDR}
ip addr add 10.0.0.${MY_ADDR}/24 dev eth0
ip link set dev eth0 up

ip link set eth1 address 00:90:01:00:00:0${MY_ADDR}
ip addr add 10.0.1.${MY_ADDR}/24 dev eth1
ip link set dev eth1 up

ip link set eth2 address 00:90:02:00:00:0${MY_ADDR}
ip addr add 10.0.2.${MY_ADDR}/24 dev eth2
ip link set dev eth2 up

ip link set eth3 address 00:90:03:00:00:0${MY_ADDR}
ip addr add 10.0.3.${MY_ADDR}/24 dev eth3
ip link set dev eth3 up

ip link set eth4 address 00:90:04:00:00:0${MY_ADDR}
ip addr add 10.0.4.${MY_ADDR}/24 dev eth4
ip link set dev eth4 up

ip link set eth5 address 00:90:05:00:00:0${MY_ADDR}
ip addr add 10.0.5.${MY_ADDR}/24 dev eth4
ip link set dev eth5 up

ip link set eth6 address 00:90:06:00:00:0${MY_ADDR}
ip addr add 10.0.6.${MY_ADDR}/24 dev eth4
ip link set dev eth6 up

echo 1 > /proc/irq/31/smp_affinity
echo 2 > /proc/irq/32/smp_affinity
echo 3 > /proc/irq/33/smp_affinity
echo 4 > /proc/irq/30/smp_affinity


if [ "$MY_RANK" == "0" ]
then
    iperf3 -s -p 5002 -B 10.0.0.2 -1 &
    iperf3 -s -p 5003 -B 10.0.1.2 -1 &
    iperf3 -s -p 5004 -B 10.0.2.2 -1 &
    iperf3 -s -p 5005 -B 10.0.3.2 -1 &
    iperf3 -s -p 5006 -B 10.0.4.2 -1 &
    iperf3 -s -p 5007 -B 10.0.5.2 -1 &
    iperf3 -s -p 5008 -B 10.0.6.2 -1

    m5 exit 1
else
    ping -c1 -I eth0 10.0.0.2
    ping -c1 -I eth1 10.0.1.2
    ping -c1 -I eth2 10.0.2.2
    ping -c1 -I eth3 10.0.3.2
    ping -c1 -I eth4 10.0.4.2
    ping -c1 -I eth5 10.0.5.2
    ping -c1 -I eth6 10.0.6.2

    sleep 0.1
    
    iperf3 -c 10.0.0.2 -t 2 -p 5002 -B 10.0.0.3 &
    iperf3 -c 10.0.1.2 -t 2 -p 5003 -B 10.0.1.3 &
    iperf3 -c 10.0.2.2 -t 2 -p 5004 -B 10.0.2.3 &
    iperf3 -c 10.0.3.2 -t 2 -p 5005 -B 10.0.3.3 &
    iperf3 -c 10.0.4.2 -t 2 -p 5006 -B 10.0.4.3 &
    iperf3 -c 10.0.5.2 -t 2 -p 5007 -B 10.0.5.3 &
    iperf3 -c 10.0.6.2 -t 2 -p 5008 -B 10.0.6.3 

    wait
    m5 exit 1
fi
