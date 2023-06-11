#!/bin/bash

CACHE_CONFIG="--caches --l2cache --l1i_size=32kB --l1d_size=32kB --l1d_assoc=8 --l2_size=1MB --l2_assoc=16"

function usage {
  echo "Usage: $0 --num-nics <num_nics> [--script <script>] [--loadgen-find-bw] [--take-checkpoint] [-h|--help]"
  echo "  --num-nics <num_nics> : number of NICs to use"
  echo "  --script <script> : guest script to run"
  echo "  --loadgen-find-bw : run loadgen in find bandwidth mode"
  echo "  --take-checkpoint : take checkpoint after running"
  echo "  -h --help : print this message"
  exit 1
}

function setup_dirs {
  mkdir -p "$CKPT_DIR"
  mkdir -p "$RUNDIR"
}

function run_simulation {
  "$GEM5_DIR/build/ARM/gem5.$GEM5TYPE" $DEBUG_FLAGS --outdir="$RUNDIR" \
  "$GEM5_DIR"/configs/example/fs.py --cpu-type=$CPUTYPE \
  --kernel="$RESOURCES/vmlinux" --disk="$RESOURCES/rootfs.ext2" --bootloader="$RESOURCES/boot.arm64" --root=/dev/sda \
  --num-cpus=2 --mem-size=8192MB --script="$GUEST_SCRIPT_DIR/$GUEST_SCRIPT" \
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
TEMP=$(getopt -o 'h' --long take-checkpoint,num-nics:,script:,packet-rate,loadgen-find-bw,help -n 'dpdk-loadgen' -- "$@")

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
  --packet-rate)
    PACKET_RATE="$3"
    shift 2
    ;;
  --loadgen-find-bw)
    LOADGENREPLYMODE="ReplayAndAdjustThroughput"
    shift 1
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

CKPT_DIR=${GIT_ROOT}/ckpts/$num_nics"NIC"-$GUEST_SCRIPT

if [[ -z "$num_nics" ]]; then
  echo "Error: missing argument --num-nics" >&2
  usage
fi

if [[ -n "$checkpoint" ]]; then
  RUNDIR=${GIT_ROOT}/rundir/$num_nics"NIC-ckp"
  setup_dirs
  echo "Taking Checkpoint for NICs=$num_nics" >&2
  GEM5TYPE="fast"
  CPUTYPE="AtomicSimpleCPU"
  CONFIGARGS="-m 6000000000000"
  run_simulation
  exit 0
else
  if [[ -z "$PACKET_RATE" ]]; then
    echo "Error: missing argument --packet_rate" >&2
    usage
  fi

  PORT=11211    # for memcached
  PCAP_FILENAME="../memcached.pcap"
  ((INCR_INTERVAL = PACKET_RATE / 10))
  RUNDIR=${GIT_ROOT}/rundir/$num_nics"NIC"
  setup_dirs
  CPUTYPE="AtomicSimpleCPU" # just because DerivO3CPU is too slow sometimes
  GEM5TYPE="opt"
  LOADGENMODE=${LOADGENMODE:-"Static"}
  DEBUG_FLAGS="--debug-flags=LoadgenDebug"
  CONFIGARGS="$CACHE_CONFIG -r 1 --loadgen-type=Pcap --loadgen_pcap_filename=$PCAP_FILENAME --loadgen-start=4050000000000 -m=4500000000000 --packet-rate=$PACKET_RATE --loadgen-replymode=$LOADGENREPLYMODE --loadgen-port-filter=$PORT --loadgen-increment-interva=$INCR_INTERVAL"
  run_simulation
  exit
fi
