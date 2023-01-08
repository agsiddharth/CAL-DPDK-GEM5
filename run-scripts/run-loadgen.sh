#1 num loadgens
#5 l1size
#6 packet rate
#7 packet size
#8 Perfect IO
#9 L2 size

let RATE=$7*$6*8/1024/1024/1024
echo Running loadgen at $RATE gbps with $7 byte packets

mkdir -p ../dpdk-rundir/$1nics-$2-$3channel-$4-l1$5-io-$8-l2$9

../gem5/build/ARM/gem5.fast --outdir=../dpdk-rundir/$1nics-$2-$3channel-$4-l1$5-io-$8/$RATE-$7BYTE -r \
../gem5/configs/example/fs.py \
--kernel=../resources/vmlinux --disk=../resources/rootfs.ext2 --bootloader=../resources/boot.arm64 --root=/dev/sda \
--checkpoint-dir=../dpdk-rundir/1nics-2GHz-2channel-AtomicSimpleCPU-l132kB-io-False/31-1518BYTE/  \
--num-cpus=2 --mem-size=8192MB --script=../guest-scripts/dpdk-1.sh \
--packet-rate=$6 --packet-size=$7 \
--num-nics=1 --num-loadgens=$1 \
--checkpoint-dir=../ckpts/2Core1NIC \
--loadgen-start=4050000000000 --loadgen-stop=4500000000000 \
-m 6000000000000 \
--caches --l2cache --l1i_size=$5 --l1d_size=$5 --l1d_assoc=2 --l2_size=$9 --l2_assoc=16 \
--mem-type=DDR4_2400_8x8 --mem-channels=$3 \
--cpu-clock=$2 \
--perf-io=$8 \
--cpu-type=$4 \
--ddio-enabled \
-r 2

## take checkpoints starting at 23 s and every second after. this should be enough time for testpmd to start
