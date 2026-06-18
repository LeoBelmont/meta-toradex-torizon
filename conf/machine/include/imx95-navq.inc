WKS_FILE:sota = "torizon-imx95-sota.wks"

OSTREE_KERNEL_ARGS:sota:imx:append = "console=ttyLP0,115200 earlycon root=LABEL=otaroot rootfstype=ext4"

UENV_EXTRA_CONFIGS = "env set ramdisk_addr_r "${initrd_addr}""

UBOOT_BOOT_PARTITION_NUMBER = "1"
OTAROOT_PARTITION_NUMBER = "1"

PREFERRED_PROVIDER_virtual/dtb = ""
PREFERRED_PROVIDER_u-boot-default-script = "u-boot-distro-boot"

BBMULTICONFIG ?= "${NAVQ_M7_MC}"

# meta-nxp-mr's python3-flit-core (3.9.0) predates PEP 639 and breaks poky's
# python3-sphinx 8.2.1; mask it so poky's PEP-639-aware flit-core is used.
BBMASK += "/meta-nxp-mr/recipes-devtools/python/python3-flit-core"

# The shared mx95-nxp-bsp splash kmods are the Verdin i.MX95 display chain, which
# NavQ's kernel doesn't build
INITRAMFS_EXTRA_KMODS:forcevariable = ""

# Workaround for build error
IMAGE_LOG_CHECK_EXCLUDES += "ctrl-alt-del\.target.*/dev/null"
