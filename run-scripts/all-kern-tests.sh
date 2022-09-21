#! /bin/bash

#1 script
#2 clock speed
#3 memory channel
#4 CPU Type
#5 l1size
#6 perfect iocache
#7 l2size

## BASELINE
./run-dist-gem5.sh iperf-1 2GHz 2 DerivO3CPU 32kB False 2MB &
./run-dist-gem5.sh iperf-2 2GHz 2 DerivO3CPU 32kB False 2MB &
./run-dist-gem5.sh iperf-3 2GHz 2 DerivO3CPU 32kB False 2MB &
./run-dist-gem5.sh iperf-4 2GHz 2 DerivO3CPU 32kB False 2MB &

# sleep 2

# ## 3GHZ
./run-dist-gem5.sh iperf-1 3GHz 2 DerivO3CPU 32kB False 2MB &
./run-dist-gem5.sh iperf-2 3GHz 2 DerivO3CPU 32kB False 2MB &
./run-dist-gem5.sh iperf-3 3GHz 2 DerivO3CPU 32kB False 2MB &
./run-dist-gem5.sh iperf-4 3GHz 2 DerivO3CPU 32kB False 2MB &

# sleep 2

# ## 3GHZ; Perfect IO Cache
./run-dist-gem5.sh iperf-1 3GHz 2 DerivO3CPU 32kB True 2MB &
./run-dist-gem5.sh iperf-2 3GHz 2 DerivO3CPU 32kB True 2MB &
./run-dist-gem5.sh iperf-3 3GHz 2 DerivO3CPU 32kB True 2MB &
./run-dist-gem5.sh iperf-4 3GHz 2 DerivO3CPU 32kB True 2MB &

# sleep 2

# ## 3GHZ; 2x MEM CHANNEL; Perfect IO Cache
./run-dist-gem5.sh iperf-1 3GHz 4 DerivO3CPU 32kB True 2MB &
./run-dist-gem5.sh iperf-2 3GHz 4 DerivO3CPU 32kB True 2MB &
./run-dist-gem5.sh iperf-3 3GHz 4 DerivO3CPU 32kB True 2MB &
./run-dist-gem5.sh iperf-4 3GHz 4 DerivO3CPU 32kB True 2MB &

# # sleep 1

# # ## 3GHZ; 2x MEM CHANNEL; Perfect IO Cache; 2x ROB
./run-dist-gem5.sh iperf-1 3GHz 4 bigROB 32kB True 2MB &
./run-dist-gem5.sh iperf-2 3GHz 4 bigROB 32kB True 2MB &
./run-dist-gem5.sh iperf-3 3GHz 4 bigROB 32kB True 2MB &
./run-dist-gem5.sh iperf-4 3GHz 4 bigROB 32kB True 2MB &

# sleep 2

# ## 3GHZ; 2x MEM CHANNEL; Perfect IO Cache; 2x ROB; 2x LSU
./run-dist-gem5.sh iperf-1 3GHz 4 bigLSU 32kB True 2MB &
./run-dist-gem5.sh iperf-2 3GHz 4 bigLSU 32kB True 2MB &
./run-dist-gem5.sh iperf-3 3GHz 4 bigLSU 32kB True 2MB &
./run-dist-gem5.sh iperf-4 3GHz 4 bigLSU 32kB True 2MB &

# sleep 2

# ## 3GHZ; 2x MEM CHANNEL; Perfect IO Cache; 2x ROB; 2x L1
./run-dist-gem5.sh iperf-1 3GHz 4 bigLSU 64kB True 2MB &
./run-dist-gem5.sh iperf-2 3GHz 4 bigLSU 64kB True 2MB &
./run-dist-gem5.sh iperf-3 3GHz 4 bigLSU 64kB True 2MB &
./run-dist-gem5.sh iperf-4 3GHz 4 bigLSU 64kB True 2MB &

sleep 2

## 3GHZ; 2x MEM CHANNEL; Perfect IO Cache; 2x ROB; 2x L1 2xL2
./run-dist-gem5.sh iperf-1 3GHz 4 bigLSU 64kB True 4MB &
./run-dist-gem5.sh iperf-2 3GHz 4 bigLSU 64kB True 4MB &
./run-dist-gem5.sh iperf-3 3GHz 4 bigLSU 64kB True 4MB &
./run-dist-gem5.sh iperf-4 3GHz 4 bigLSU 64kB True 4MB &

sleep 2

## 3GHZ; 2x MEM CHANNEL; Perfect IO Cache; 2x ROB; 2x L1; SCALING
./run-dist-gem5.sh iperf-5 3GHz 4 bigLSU 64kB True 4MB &
./run-dist-gem5.sh iperf-6 3GHz 4 bigLSU 64kB True 4MB &
./run-dist-gem5.sh iperf-7 3GHz 4 bigLSU 64kB True 4MB &