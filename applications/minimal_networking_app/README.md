### The minimal DPDK networking app for gem5

This is the **minimal** user application demonstrating networking I/O with DPDK in gem5. This app can be used as a starting point to develop kernel-bypass newtworking software in gem5. This has been tested with the following configuration:

- Arch: `aarch64`
- Kernel: `5.10.7`
- NIC: `Intel i8254xGBe`
- driver: `uio_pci_generic`
- DPDK version: `20.11.3`

### How to build it and run

For now, this example is NOT included in the buildroot for the rootfs for this model. It should be built separately with a cross-compilation aarch64 toolchain, and then deployed in the model with the secondary disk image. Due to certain cautions required to *properly* build and link cross-compiled DPDK applicaitons, the current build is based on a hand-writen Makefile.

##### Prerequisites

* aarch64 toolchain for Linux: `https://toolchains.bootlin.com/releases_aarch64.html`; for current gem5 Linux environment, use the version with GCC 10.2.0, GLIBC 2.31, i.e. `https://toolchains.bootlin.com/downloads/releases/toolchains/aarch64/tarballs/aarch64--glibc--bleeding-edge-2020.08-1.tar.bz2`; Alternatively, one sould also be able to use the default toolchain from the buldroot used to build Kernel and Disk Image for simulation (not tested);
* squashfs

##### Build

* set env variables:
** `AARCH64_TOOLCHAIN_PATH` to point to the aarch64 toolchain;
** `RTE_SDK` to point to the DPDK build folder used in the current gem5 buldroot image;
** `GEM5_FS_LIB_PATH` to point to the `/use/lib` folder of the *mounted current* rootfs image; this is optional, as all required DPDK libraries can be found in `RTE_SDK`, but for some reason, linking against the libraries *actually* installed in the simulation speeds-up application start-up by ~x10
* make

This will generate the `userfs.sqsh` disk image which contains the app binary.

##### Run

* Run simulation with the secondary disk image pointing to the generated `userfs.sqsh` by providing an extra disk image flag in `run_simulation` in the simulation script, i.e. `--disk="<PATH_TO_THIS_FOLDER>/userfs.sqsh"`
* after Linux is booted, go to simulation (e.g. `./m5term localhost 3456`) and mount `userfs.sqsh` with `mount -o ro /dev/sdb /mnt/userfs`
* execute the app: `/mnt/userfs/sample_app <number of packets to receive/forward> <burst size> <enable traffic logs> <enable synthetic simulation of packet processing>`

### TODOs
* use `glog`/`gflags` in the app (requires some extra steps to setup the rootfs image);
* integreate with the rootfs buildroot flow to make it easier;
* have not tried yet with `-O3`, but should work..

