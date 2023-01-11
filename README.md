# CAL-DPDK-GEM5

This repository contains the code for running kernel bypass networking (DPDK) on the gem5 system simulator, with an aritifical network load generator. 
The disk image and kernel binary is included in the repository using git LFS. (Before cloning, make sure you install git-lfs in your system).

Using VSCode devcontainers is recommended, but not required. 

If not using devcontainer, please manuall set enviornment variable GIT_ROOT to the root of this repoistory before proceeding.

```
export GIT_ROOT=<PATH_TO_REPO>
```

build gem5, both .fast and .opt varities for the ARM ISA (aarch64).

```
cd gem5
scons build/ARM/gem5.fast
scons build/ARM/gem5.opt
```

To run network benchmarks with DPDK using testpmd, we first take a checkpoint with the AtomicSimpleCPU. 

```
./l2fwd-ckp.sh --take-checkpoint --num-nics 1 --script dpdk-1.sh
```


The script will take 2 checkpoints, one after booting linux and one after setting up dpdk. To restore from the second checkpoint and find the maximum sustainable bandwidth

```
./l2fwd-ckp.sh--loadgen-find-bw --num-nics 1 --script dpdk-1.sh --packet-rate 2150786 --packet-size=1514
```

The above script will start the network load generator at 24 Gbps and increment the rate by 0.5 Gbps for every 1000 packets forwarded without loss.
The network rate will stop incrementing once packet loss is encountred. 

The core can be customized by standard gem5 fs.py args.
