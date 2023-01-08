#!/bin/bash

function usage {
  echo "Usage: $0 --num-nics <num_nics> [--take-checkpoint]"
  echo "  --num-nics <num_nics> : number of NICs to use"
  echo "  --take-checkpoint : take checkpoint after running"
  exit 1
}

if [[ -z "${GIT_ROOT}" ]]; then
    echo "Please export env var GIT_ROOT to point to the root of the CAL-DPDK-GEM5 repo"
    exit 1
fi

# parse command line arguments
TEMP=$(getopt -o '' --long take-checkpoint,num-nics: -n 'dpdk-loadgen' -- "$@")

# check for parsing errors
if [ $? != 0 ] ; then 
  echo "Error: unable to parse command line arguments" >&2 
  exit 1
fi

eval set -- "$TEMP"
while true; do
  case "$1" in
    --num-nics ) num_nics="$2"; shift 2;;
    --take-checkpoint ) checkpoint=1; shift 2;;
    -- ) shift; break ;;
    * ) break ;;
  esac
done

GEM5_DIR=${GIT_ROOT}/gem5
RESOURCES=${GIT_ROOT}/resources
GUEST_SCRIPTS=${GIT_ROOT}/guest-scripts
CKPT_DIR=${GIT_ROOT}/ckpts/$num_nics"NIC"
RUNDIR=${GIT_ROOT}/rundir/$num_nics"NIC"

if [[ -z "$num_nics" ]]; then
  echo "Error: missing argument --num-nics" >&2
  usage
fi

if [[ -n "$checkpoint" ]]; then
    RUNDIR=${GIT_ROOT}/rundir/$num_nics"NIC-ckp"
  echo "Taking Checkpoint for NICs=$num_nics" >&2
fi

mkdir -p "$CKPT_DIR"
mkdir -p "$RUNDIR"

"$GEM5_DIR"/build/ARM/gem5.fast --outdir="$RUNDIR" \
"$GEM5_DIR"/configs/example/fs.py --cpu-type=AtomicSimpleCPU \
--kernel="$RESOURCES/vmlinux" --disk="$RESOURCES/rootfs.ext2" --bootloader="$RESOURCES/boot.arm64" --root=/dev/sda \
--num-cpus="$num_nics" --mem-size=8192MB --script="$GUEST_SCRIPTS/dpdk-$num_nics.sh" \
--num-nics="$num_nics" --num-loadgens="$num_nics" \
--checkpoint-dir="$CKPT_DIR" \
-m 6000000000000 
