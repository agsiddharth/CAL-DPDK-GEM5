#!/bin/ash

# Retrieve dist-gem5 rank and size parameters using magic keys
MY_RANK=$(m5 initparam dist-rank)
MY_SIZE=$(m5 initparam dist-size)

/bin/hostname tux"${MY_RANK}"

MY_ADDR=$((MY_RANK + 2)) 

ip link set eth0 address 00:90:00:00:00:0${MY_ADDR}
ip addr add 10.0.0.${MY_ADDR}/24 dev eth0
ip link set dev eth0 up

ip link set eth1 address 00:90:01:00:00:0${MY_ADDR}
ip addr add 10.0.1.${MY_ADDR}/24 dev eth1
ip link set dev eth1 up

echo 1 > /proc/irq/31/smp_affinity
echo 2 > /proc/irq/32/smp_affinity

if [ "$MY_RANK" = "0" ]
then
    iperf3 -s -p 5002 -B 10.0.0.2 -1 &
    iperf3 -s -p 5003 -B 10.0.1.2 -1

    m5 exit 1
else
    ping -c1 -I eth0 10.0.0.2
    ping -c1 -I eth1 10.0.1.2

    sleep 0.1
    iperf3 -c 10.0.0.2 -t 2 -p 5002 -B 10.0.0.3 &
    iperf3 -c 10.0.1.2 -t 2 -p 5003 -B 10.0.1.3
    
    wait
    m5 exit 1
fi
