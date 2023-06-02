DPDK_VERSION:=20.11.3
DPDK_SITE:="$(BR2_EXTERNAL_DPDK_GEM5_PATH)/package/dpdk/dpdk-source"
DPDK_SITE_METHOD:=local
DPDK_INSTALL_TARGET:=YES
DPDK_CONF_OPTS += -Dexamples=all
$(eval $(meson-package))