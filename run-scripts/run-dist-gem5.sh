#! /bin/bash

#1 script
#2 clock speed
#3 memory channel
#4 CPU Type
#5 l1size
#6 perf iocache
#7 l2size

GEM5_DIR=$(pwd)/../gem5

IMG=$(pwd)/../resources/rootfs.ext2
VMLINUX=$(pwd)/../resources/vmlinux

FS_CONFIG=$GEM5_DIR/configs/example/fs.py
SW_CONFIG=$GEM5_DIR/configs/dist/sw.py

GEM5_EXE=$GEM5_DIR/build/ARM/gem5.opt

SCRIPT=$(pwd)/../guest-scripts/$1.sh
NNODES=2

$(pwd)/gem5-dist-condor.sh -n $NNODES                               \
                -r $(pwd)/../rundir/$1-$2-$3channel-$4-l1$5-io-$6-l2$7 -c $(pwd)/../dist-ckpts/7core-ckpt          \
                -s $SW_CONFIG                  \
                -f $FS_CONFIG                  \
                -x $GEM5_EXE                   \
                --fs-args                      \
                    --kernel=$VMLINUX          \
                    --script=$SCRIPT           \
                    --bootloader $(pwd)/../resources/boot.arm64                \
                    --num-cpus=8                                            \
                    --disk-image=$IMG                                          \
                    --mem-size=8GB --mem-type=DDR4_2400_8x8 --mem-channels=$3  \
                    --cpu-type=$4                                       \
                    --caches --l2cache --l1i_size=$5 --l1d_size=$5 --l1d_assoc=2 --l2_size=$7 --l2_assoc=16 \
                    --num-nics=7                                               \
                    --cpu-clock=$2  \
                    --root=/dev/sda  \
                    --perf-io=$6  \
                    --checkpoint-restore=1 
