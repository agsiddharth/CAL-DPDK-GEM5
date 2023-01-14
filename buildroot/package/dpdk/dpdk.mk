DPDK_VERSION:=1.0.0
DPDK_SITE:="$(BR2_EXTERNAL_DPDK_GEM5_PATH)/package/dpdk/dpdk-source"
DPDK_SITE_METHOD:=local
DPDK_INSTALL_TARGET:=YES

$(eval $(meson-package))