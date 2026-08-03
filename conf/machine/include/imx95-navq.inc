WKS_FILE:sota = "torizon-imx95-sota.wks"

OSTREE_KERNEL_ARGS:sota:imx:append = "console=ttyLP0,115200 earlycon root=LABEL=otaroot rootfstype=ext4"

UENV_EXTRA_CONFIGS = "env set ramdisk_addr_r "${initrd_addr}""

UBOOT_BOOT_PARTITION_NUMBER = "1"
OTAROOT_PARTITION_NUMBER = "1"

PREFERRED_PROVIDER_virtual/dtb = ""
PREFERRED_PROVIDER_u-boot-default-script = "u-boot-distro-boot"

# NavQ builds none of the mx95-nxp-bsp splash kmods; :forcevariable beats that override.
INITRAMFS_EXTRA_KMODS:forcevariable = ""

# preset-all's benign refusal to clobber the ctrl-alt-del mask must not fail log_check.
IMAGE_LOG_CHECK_EXCLUDES += "ctrl-alt-del\.target.*/dev/null"

# Cortex-M7 firmware. To build without it also BBMASK meta-nxp-mr's imx-mkimage.
BBMULTICONFIG ?= "${NAVQ_M7_MC}"
