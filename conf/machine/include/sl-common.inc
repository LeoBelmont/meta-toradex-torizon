PREFERRED_PROVIDER_u-boot-fw-utils = "libubootenv"
PREFERRED_RPROVIDER_u-boot-fw-utils = "libubootenv"
PREFERRED_PROVIDER_u-boot-default-script = "u-boot-distro-boot"
PREFERRED_PROVIDER_u-boot-default-script:sota = "u-boot-distro-boot"
IMAGE_BOOT_FILES:append = " boot.scr-${MACHINE};boot.scr"
WKS_FILE_DEPENDS:append = " u-boot-default-script"

OTAROOT_PARTITION_NUMBER = "${UBOOT_BOOT_PARTITION_NUMBER}"
export UBOOT_BOOT_PARTITION_NUMBER

OSTREE_DEVICETREE = " \
    ${SYNA_KERNEL_DTS_FILE} \
    ${SYNA_KERNEL_DTBO_FILE} \
"

BBMASK += "/meta-synaptics/dynamic-layers/virtualization-layer/recipes-containers/docker/docker-moby_git.bbappend"

PREFERRED_PROVIDER_virtual/dtb = ""

IMAGE_CMD:ota-ext4:append () {
    # Align ROOTFS_SIZE to 4KB (4096 bytes) to avoid img2simg assertion errors
    ROOTFS_SIZE_ALIGNED=$(echo "scale=0; ((${ROOTFS_SIZE} + 4095) / 4096) * 4096" | bc)
    synaimg_mkext234fs ${OTA_SYSROOT} rootfs_ota ext4 ${ROOTFS_SIZE_ALIGNED} ${EXTRA_IMAGECMD}
}

WKS_FILE:sota = "sl-common.wks"

IMAGE_FSTYPES:append = " wic"
IMAGE_FSTYPES:remove:pn-core-image-initramfs-boot = "wic"

syna_flash_package() {
    IMG="${IMGDEPLOYDIR}/${IMAGE_NAME}.wic"
    [ -f "$IMG" ] || bbfatal "WIC image not found at $IMG"

    FLASH_DIR="${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}-flash"
    install -d "$FLASH_DIR"
    cp "$IMG" "$FLASH_DIR/emmc.wic"

    cp -L -R ${DEPLOY_DIR_IMAGE}/syna-metadata/* "$FLASH_DIR/"
    cp -L -R ${DEPLOY_DIR_IMAGE}/syna-usb-tool/* "$FLASH_DIR/"

    # WIC writes only the eMMC user area; preboot lives in eMMC hw boot
    # partitions. We need to include/flash it separetely.
    cp "${DEPLOY_DIR_IMAGE}/preboot.subimg" "$FLASH_DIR/"

    # Create a SYNAIMG-flash.tar.gz file with the WIC image, usb-tool and recovery folder inside.
    tar -czf "${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}-flash.tar.gz" \
        -C "${DEPLOY_DIR_IMAGE}" "${SYNAIMG_DEPLOY_SUBDIR}-flash"
}

do_image_wic[postfuncs] += "syna_flash_package"

do_image_wic[depends] += " \
    syna-metadata:do_deploy \
    syna-usb-tool:do_deploy \
"

do_image_wic[depends] += "${PN}:do_image_ota_ext4"

# Ensure do_image_synaimg runs after do_image_ostree completes since both copy from IMAGE_ROOTFS,
# and if they run simultaneously it can cause FileNotFoundError on temporary files created by rsync/tar.
do_image_synaimg[depends] += "${PN}:do_image_ostree"
