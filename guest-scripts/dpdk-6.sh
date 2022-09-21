
ip link set dev eth0 down
modprobe uio_pci_generic
dpdk-devbind.py -b uio_pci_generic 00:02.0
dpdk-devbind.py -b uio_pci_generic 00:03.0
dpdk-devbind.py -b uio_pci_generic 00:04.0
dpdk-devbind.py -b uio_pci_generic 00:05.0
dpdk-devbind.py -b uio_pci_generic 00:06.0
dpdk-devbind.py -b uio_pci_generic 00:07.0

echo 2048 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
m5 checkpoint
dpdk-testpmd --nb-cores=6
