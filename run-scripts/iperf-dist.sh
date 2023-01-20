#!/bin/bash

GEM5_DIR=${GIT_ROOT}/gem5
RESOURCES=${GIT_ROOT}/resources
GUEST_SCRIPT_DIR=${GIT_ROOT}/guest-scripts


FS_CONFIG=$GEM5_DIR/configs/example/fs.py
SW_CONFIG=$GEM5_DIR/configs/dist/sw.py
GEM5_EXE=$GEM5_DIR/build/ARM/gem5.fast

function run_dist_simulation {
    "$GEM5_DIR"/util/dist/gem5-dist.sh -n 2 \
    -r "$RUNDIR" -c "$CKPT_DIR" \
    -s "$SW_CONFIG" -f "$FS_CONFIG" -x "$GEM5_EXE" \
    --fs-args \
    --cpu-type=$CPUTYPE \
    --kernel="$RESOURCES/vmlinux" --disk="$RESOURCES/rootfs.ext2" --bootloader="$RESOURCES/boot.arm64" --root=/dev/sda \
    --num-cpus=1 --mem-size=8192MB --script="$GUEST_SCRIPT_DIR/$GUEST_SCRIPT" \
    --num-nics="$num_nics" \
    $CONFIGARGS
}

function setup_dirs {
  mkdir -p "$CKPT_DIR"
  mkdir -p "$RUNDIR"
}

function usage {
  echo "Usage: $0 --num-nics <num_nics> [--script <script>] [--take-checkpoint] [-h|--help]"
  echo "  --num-nics <num_nics> : number of NICs to use"
  echo "  --script <script> : guest script to run"
  echo "  --take-checkpoint : take checkpoint after running"
  echo "  -h --help : print this message"
  exit 1
}

# parse command line arguments
TEMP=$(getopt -o 'h' --long take-checkpoint,num-nics:,script:,help -n 'iperf-dist' -- "$@")

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
    GUEST_SCRIPT="$2"
    shift 2
    ;;
  -h | --help)
    usage
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

CKPT_DIR=${GIT_ROOT}/ckpts/$num_nics"NIC"-$GUEST_SCRIPT


if [[ -n "$checkpoint" ]]; then
  RUNDIR=${GIT_ROOT}/rundir/$num_nics"NIC-ckp"-${GUEST_SCRIPT}
  setup_dirs
  echo "Taking Checkpoint for NICs=$num_nics" >&2
  CPUTYPE="AtomicSimpleCPU"
  CONFIGARGS="-m 832240000000"
  run_dist_simulation
  exit 0
else
  if [[ -z "$PACKET_SIZE" ]]; then
    echo "Error: missing argument --packet_size" >&2
    usage
  fi

  if [[ -z "$PACKET_RATE" ]]; then
    echo "Error: missing argument --packet_rate" >&2
    usage
  fi
  RUNDIR=${GIT_ROOT}/rundir/$num_nics"NIC"-${GUEST_SCRIPT}
  setup_dirs
  CPUTYPE="DerivO3CPU"
  CONFIGARGS="$CACHE_CONFIG -r 1"
  run_dist_simulation
  exit
fi