#!/bin/bash 

## list of packet rates to test
rates=(5000 10000 20000 30000 40000)
##l2 sizes
l2_sizes=(256kB 512kB 4MB 8MB)
## frequencies
freq=(1GHz 2GHz 3GHz 4GHz)

## loop over all packet rates
for rate in "${rates[@]}"; do
    ## loop over all frequencies
    for f in "${freq[@]}"; do
      ## run the experiment
      ./memcached-kernel.sh --num-nics 1 --script memcached_kernel.sh --packet-rate $rate --l2-size 1MB --freq $f &
  done
done

## loop over all packet rates and then l2 sizes
for rate in "${rates[@]}"; do
    for l2 in "${l2_sizes[@]}"; do
      ./memcached-kernel.sh --num-nics 1 --script memcached_kernel.sh --packet-rate $rate --l2-size $l2 --freq 3GHz &
  done
done

wait