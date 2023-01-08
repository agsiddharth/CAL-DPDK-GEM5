#!/bin/bash

CACHE_CONFIG="--caches --l2cache --l1i_size=32kB --l1d_size=32kB --l1d_assoc=8 --l2_size=1MB --l2_assoc=16"

function usage {
  echo "Usage: $0 --num-nics <num_nics> [--take-checkpoint] [--script <script>]"
  echo "  --num-nics <num_nics> : number of NICs to use"
  echo "  --take-checkpoint : take checkpoint after running"
  echo "  --script <script> : guest script to run"
  exit 1
}

function setup_dirs {
  mkdir -p "$CKPT_DIR"
  mkdir -p "$RUNDIR"
}

function run_simulation {
  "$GEM5_DIR"/build/ARM/gem5.fast --outdir="$RUNDIR" \
    "$GEM5_DIR"/configs/example/fs.py --cpu-type=$CPUTYPE \
    --kernel="$RESOURCES/vmlinux" --disk="$RESOURCES/rootfs.ext2" --bootloader="$RESOURCES/boot.arm64" --root=/dev/sda \
    --num-cpus=2 --mem-size=8192MB --script="$GUEST_SCRIPT" \
    --num-nics="$num_nics" --num-loadgens="$num_nics" \
    --checkpoint-dir="$CKPT_DIR" $CONFIGARGS
}

if [[ -z "${GIT_ROOT}" ]]; then
  echo "Please export env var GIT_ROOT to point to the root of the CAL-DPDK-GEM5 repo"
  exit 1
fi

GEM5_DIR=${GIT_ROOT}/gem5
RESOURCES=${GIT_ROOT}/resources
GUEST_SCRIPT_DIR=${GIT_ROOT}/guest-scripts

# parse command line arguments
TEMP=$(getopt -o '' --long take-checkpoint,num-nics:,script:,packet-rate:,packet-size: -n 'dpdk-loadgen' -- "$@")

# check for parsing errors
if [ $? != 0 ]; then
  echo "Error: unable to parse command line arguments" >&2
  exit 1
fi

eval set -- "$TEMP"

while true; do
  case "$1" in
  --num-nics)
    num_nics="$2"
    shift 2
    ;;
  --take-checkpoint)
    checkpoint=1
    shift 1
    ;;
  --script)
    GUEST_SCRIPT="$GUEST_SCRIPT_DIR/$2"
    shift 2
    ;;
  --packet-size)
    PACKET_SIZE="$2"
    shift 2
    ;;
  --packet-rate)
    PACKET_RATE="$2"
    shift 2
    ;;
  --)
    shift
    break
    ;;
  *) break ;;
  esac
done

if [[ -z "$num_nics" ]]; then
  echo "Error: missing argument --num-nics" >&2
  usage
fi

CKPT_DIR=${GIT_ROOT}/ckpts/$num_nics"NIC"

if [[ -n "$checkpoint" ]]; then
  RUNDIR=${GIT_ROOT}/rundir/$num_nics"NIC-ckp"
  setup_dirs
  echo "Taking Checkpoint for NICs=$num_nics" >&2
  CPUTYPE="AtomicSimpleCPU"
  CONFIGARGS="-m 6000000000000"
  run_simulation
  exit 0
else
  RUNDIR=${GIT_ROOT}/rundir/$num_nics"NIC"
  setup_dirs
  ((RATE = PACKET_RATE * PACKET_SIZE * 8 / 1024 / 1024 / 1024))
  echo "Running NICs=$num_nics at $RATE GBPS" >&2
  CPUTYPE="DerivO3CPU"
  CONFIGARGS="$CACHE_CONFIG -r 2 --loadgen-start=6050000000000 --loadgen-stop=11500000000000 --packet-rate=$RATE --packet-size=$PACKET_SIZE --loadgen-mode=static"
  run_simulation
  exit
fi
