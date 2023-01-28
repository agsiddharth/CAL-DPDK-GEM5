# CAL-DPDK-GEM5

This repository contains the code for running kernel bypass networking (DPDK) on the gem5 system simulator, with an artificial network load generator.

Check out our paper for more information:

>Siddharth Agarwal, Minwoo Lee, Ren Wang, and Mohammad Alian.
>"[**Enabling Kernel Bypass Networking on gem5**](https://arxiv.org/abs/2301.09470)".
>arxiv , January 2023.



The disk image and kernel binary is included in the repository using git LFS. (Before cloning, make sure you install git-lfs in your system).

Using VSCode devcontainers is recommended, but not required.

If not using devcontainers, please manually set environment variable GIT_ROOT to the root of this repository before proceeding.

```bash
export GIT_ROOT=<PATH_TO_REPO>
```

build gem5, both .fast and .opt varieties for the ARM ISA (aarch64).

```bash
cd gem5
scons build/ARM/gem5.fast
scons build/ARM/gem5.opt
```

To run network benchmarks with DPDK using testpmd, we first take a checkpoint with the AtomicSimpleCPU.

```bash
./l2fwd-ckp.sh --take-checkpoint --num-nics 1 --script dpdk-1.sh
```

The script will take 2 checkpoints, one after booting linux and one after setting up dpdk. To restore from the second checkpoint and find the maximum sustainable bandwidth

```bash
./l2fwd-ckp.sh--loadgen-find-bw --num-nics 1 --script dpdk-1.sh --packet-rate 2150786 --packet-size=1514
```

The above script will start the network load generator at 24 Gbps and increment the rate by 0.5 Gbps for every 1000 packets forwarded without loss.
The network rate will stop incrementing once packet loss is encountered.

The core can be customized by standard gem5 fs.py args.

## Appendix

Building Kernel and Disk Image:

The repository also contains the modified source code for DPDK, and the appropriate linux config for gem5.
We use the buildroot tool to build both the linux image and the filesystem. DPDK is also built as a buildroot package.

```bash
cd /buildroot               # or your buildroot install path if not using the devcontainer
make BR2_EXTERNAL=$GIT_ROOT/buildroot gem5_defconfig && make
cp /buildroot/output/images/rootfs.ext2 $GIT_ROOT/resources/
cp /buildroot/output/images/vmlinux $GIT_ROOT/resources/
```

## Citation Information

If you are using the DPDK extension on gem5 in your work, please cite our paper:

```
@misc{https://doi.org/10.48550/arxiv.2301.09470,
  doi = {10.48550/ARXIV.2301.09470},
  url = {https://arxiv.org/abs/2301.09470},
  author = {Agarwal, Siddharth and Lee, Minwoo and Wang, Ren and Alian, Mohammad},
  keywords = {Hardware Architecture (cs.AR), Networking and Internet Architecture (cs.NI), FOS: Computer and information sciences, FOS: Computer and information sciences, C.4},
  title = {Enabling Kernel Bypass Networking on gem5},
  publisher = {arXiv},
  year = {2023},
  copyright = {Creative Commons Attribution 4.0 International}
}
```
