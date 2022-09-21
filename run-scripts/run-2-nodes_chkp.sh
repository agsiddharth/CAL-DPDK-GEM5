#! /bin/bash
export M5_PATH=/Users/sinjongmin/research/IDIO/dpdk-gem5/
echo $M5_PATH
GEM5_DIR=$(pwd)/../gem5

IMG=$(pwd)/../resources/rootfs.ext2
IMG=$(pwd)/../resources/ubuntu-18.04-arm64-docker.img
IMG=$(pwd)/../resources/mcn_aarch64.img
VMLINUX=$(pwd)/../resources/vmlinux
VMLINUX=$(pwd)/../binaries/vmlinux.arm64

FS_CONFIG=$GEM5_DIR/configs/example/fs.py
SW_CONFIG=$GEM5_DIR/configs/dist/sw.py

GEM5_EXE=$GEM5_DIR/build/ARM/gem5.opt

SCRIPT=$GEM5_DIR/util/dist/apache-bench/run.ab.rcS
#SCRIPT=$(pwd)/../guest-scripts/chkp.rcS
SCRIPT=$(pwd)/../guest-scripts/nothing.rcS
NNODES=2

DEBUG_FLAGS=" --debug-flags=DistEthernet,AdaptiveDdioOtf,AdaptiveDdioBridgeHint,AdaptiveDdioOtfInfo,AdaptiveDdioMlcPrefetcher,AdaptiveDdioXBar,AdaptiveDdioCache,AdaptiveDdioBridge,AdaptiveDdioBridgeHint "

cpu=AtomicSimpleCPU

#opts_cache=" --caches --l2cache --l2_size=1MB --l2_assoc=16 --l3cache --l3_size=4MB --l3_assoc=16  --num-cpus=4 "
opts_cpu_mem=" --cpu-type=$cpu --cpu-clock=4GHz --sys-clock=2GHz --mem-type=DDR4_2400_4x16  --mem-channels=2 "
opts_ethernet=" --ethernet-linkdelay=100ns --ethernet-linkspeed=40Gbps "
normal_ddio=" --ddio-enabled "
idio=" --ddio-enabled --mlc-adaptive-ddio  --send-prefetch-hint=1 "
header_only_for_idio=" --send-header-only=1 "
dma=""
#opts_ddio="$idio"
#rootset="--root-device="/dev/sda""
#usegen_t="--use-loadgenerator"

dtb=/Users/sinjongmin/research/IDIO/dpdk-gem5/test-land/system.dtb

opts="$opts_cache $opts_cpu_mem $opts_ethernet $opts_ddio "

distsync=" --dist-sync-start=1000000000000t"

CKPTDIR=/Users/sinjongmin/research/IDIO/dpdk-gem5/test-land/checkland
RUNDIR=/Users/sinjongmin/research/IDIO/dpdk-gem5/test-land/runland

GEM5_DIST_SH=$GEM5_DIR/util/dist/gem5-dist.sh

$GEM5_DIST_SH -n $NNODES                                                     \
              -x $GEM5_EXE                                                   \
              -c $CKPTDIR                                                    \
              -r $RUNDIR                                                     \
              -s $SW_CONFIG                                                  \
              -f $FS_CONFIG                                                  \
              --m5-args                                                      \
                 $DEBUG_FLAGS                                                \
                --fs-args                      \
                    --kernel=$VMLINUX          \
                    --machine-type=VExpress_GEM5_V1 \
                    --disk=$IMG                \
                    --script=$SCRIPT           \
                    $rootset    $opts \
                --cf-args $distsync -r1