# Example env:
# AARCH64_TOOLCHAIN_PATH := <home>/aarch64_toolchain/aarch64--glibc--bleeding-edge-2020.08-1
# RTE_SDK := <home>/buildroot/output/build/dpdk-20.11.3
# GEM5_FS_LIB_PATH := <home>/gem5_mnt/usr/lib

CC := ${AARCH64_TOOLCHAIN_PATH}/bin/aarch64-linux-g++

CFLAGS := -I${RTE_SDK}/lib/librte_eal/include \
	-I${RTE_SDK}/build \
	-I${RTE_SDK}/config \
	-I${RTE_SDK}/lib/librte_eal/linux/include \
	-I${RTE_SDK}/lib/librte_ethdev \
	-I${RTE_SDK}/lib/librte_net \
	-I${RTE_SDK}/lib/librte_eal/arm/include \
	-I${RTE_SDK}/lib/librte_mbuf \
	-I${RTE_SDK}/lib/librte_mempool \
	-I${RTE_SDK}/lib/librte_ring

LDFLAGS := -L${GEM5_FS_LIB_PATH} \
	-L${RTE_SDK}/build/drivers \
	-fno-inline-atomics \
	-lpthread \
	-Wl,--whole-archive -l:librte_flow_classify.a \
	-Wl,--whole-archive -l:librte_pipeline.a \
	-Wl,--whole-archive -l:librte_pdump.a \
	-Wl,--whole-archive -l:librte_distributor.a \
	-Wl,--whole-archive -l:librte_ip_frag.a \
	-Wl,--whole-archive -l:librte_meter.a  \
	-Wl,--whole-archive -l:librte_lpm.a  \
	-Wl,--whole-archive -l:librte_acl.a  \
	-Wl,--whole-archive -l:librte_jobstats.a  \
	-Wl,--whole-archive -l:librte_metrics.a  \
	-Wl,--whole-archive -l:librte_bitratestats.a  \
	-Wl,--whole-archive -l:librte_latencystats.a  \
	-Wl,--whole-archive -l:librte_efd.a  \
	-Wl,--whole-archive -l:librte_bpf.a  \
	-Wl,--whole-archive -l:librte_cfgfile.a  \
	-Wl,--whole-archive -l:librte_gro.a  \
	-Wl,--whole-archive -l:librte_gso.a  \
	-Wl,--whole-archive -l:librte_hash.a  \
	-Wl,--whole-archive -l:librte_member.a  \
	-Wl,--whole-archive -l:librte_vhost.a  \
	-Wl,--whole-archive -l:librte_kvargs.a  \
	-Wl,--whole-archive -l:librte_mbuf.a  \
	-Wl,--whole-archive -l:librte_net.a  \
	-Wl,--whole-archive -l:librte_ethdev.a   \
	-Wl,--whole-archive -l:librte_bbdev.a   \
	-Wl,--whole-archive -l:librte_cryptodev.a   \
	-Wl,--whole-archive -l:librte_security.a   \
	-Wl,--whole-archive -l:librte_compressdev.a   \
	-Wl,--whole-archive -l:librte_eventdev.a   \
	-Wl,--whole-archive -l:librte_rawdev.a   \
	-Wl,--whole-archive -l:librte_timer.a   \
	-Wl,--whole-archive -l:librte_mempool_ring.a   \
	-Wl,--whole-archive -l:librte_mempool.a   \
	-Wl,--whole-archive -l:librte_ring.a   \
	-Wl,--whole-archive -l:librte_pci.a   \
	-Wl,--whole-archive -l:librte_eal.a   \
	-Wl,--whole-archive -l:librte_cmdline.a   \
	-Wl,--whole-archive -l:librte_reorder.a  \
	-Wl,--whole-archive -l:librte_sched.a   \
	-Wl,--whole-archive -l:librte_common_cpt.a   \
	-Wl,--whole-archive -l:librte_common_octeontx.a   \
	-Wl,--whole-archive -l:librte_common_dpaax.a   \
	-Wl,--whole-archive -l:librte_bus_pci.a   \
	-Wl,--whole-archive -l:librte_bus_vdev.a   \
	-Wl,--whole-archive -l:librte_net_e1000.a \
	-Wl,--whole-archive -l:librte_net_tap.a \
	-Wl,--whole-archive -l:librte_net_virtio.a \
	-Wl,--whole-archive -l:librte_net_igc.a \
	-Wl,--whole-archive -l:librte_common_iavf.a \
	-Wl,--whole-archive -l:librte_kni.a \
	-Wl,--whole-archive -l:librte_telemetry.a \
	-Wl,--whole-archive -l:librte_rcu.a \
	-Wl,--no-whole-archive -l:librte_table.a \
	-Wl,--no-whole-archive -l:librte_port.a \
	-Wl,--no-whole-archive -ldl -lm -latomic

userfs: sample_app
	rm -f -r out/
	rm userfs.sqsh
	mkdir out/
	cp sample_app out/
	mksquashfs out/ userfs.sqsh

sample_app: sample_app.cc
	$(CC) -O3 $(CFLAGS) sample_app.cc $(LDFLAGS) -o sample_app

clean:
	rm sample_app
	rm -r out/
	rm userfs.sqsh
