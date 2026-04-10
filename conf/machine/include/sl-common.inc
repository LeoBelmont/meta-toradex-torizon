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
    # Clean up
    [ -f "${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}/rootfs.subimg" ] && rm ${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}/rootfs.subimg
    [ -f "${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}/rootfs_s.subimg.0" ] && rm ${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}/rootfs_s.subimg.*

    # Align ROOTFS_SIZE to 4KB (4096 bytes) to avoid img2simg assertion errors
    ROOTFS_SIZE_ALIGNED=$(echo "scale=0; ((${ROOTFS_SIZE} + 4095) / 4096) * 4096" | bc)
    synaimg_mkext234fs ${OTA_SYSROOT} rootfs_ota ext4 ${ROOTFS_SIZE_ALIGNED} ${EXTRA_IMAGECMD}

    # Convert rootfs_ota.subimg to sparse image with 300MB size limit per sub-image
    img2simg ${DEPLOY_DIR_IMAGE}/rootfs_ota.subimg ${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}/rootfs_ota_s.subimg 4096
    simg2simg ${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}/rootfs_ota_s.subimg ${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}/rootfs_ota_s.subimg 314572800

    # Prepare emmc_part_list and emmc_image_list for SYNAIMG. We make copies first to avoid modifying the original files and conflicting on subsequent builds
    cp ${DEPLOY_DIR_IMAGE}/emmc_part_list ${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}/emmc_part_list
    EMMC_PART_LIST_FILE="${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}/emmc_part_list"

    cp ${DEPLOY_DIR_IMAGE}/emmc_image_list ${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}/emmc_image_list
    EMMC_IMAGE_LIST_FILE="${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}/emmc_image_list"

    # Remove unused partitions from emmc_part_list and copy to SYNAIMG folder
    sed -i '/key_b/d' ${EMMC_PART_LIST_FILE}
    sed -i '/tzk_b/d' ${EMMC_PART_LIST_FILE}
    sed -i '/bl_b/d' ${EMMC_PART_LIST_FILE}
    sed -i '/boot_a/d' ${EMMC_PART_LIST_FILE}
    sed -i '/boot_b/d' ${EMMC_PART_LIST_FILE}
    sed -i '/firmware_b/d' ${EMMC_PART_LIST_FILE}
    sed -i '/rootfs_b/d' ${EMMC_PART_LIST_FILE}
    sed -i '/fastlogo_a/d' ${EMMC_PART_LIST_FILE}
    sed -i '/fastlogo_b/d' ${EMMC_PART_LIST_FILE}
    sed -i '/devinfo/d' ${EMMC_PART_LIST_FILE}
    sed -i '/misc/d' ${EMMC_PART_LIST_FILE}
    sed -i '/home/d' ${EMMC_PART_LIST_FILE}

    # Re-create emmc_image_list and copy to SYNAIMG folder
    cat <<EOF >${EMMC_IMAGE_LIST_FILE}
preboot.subimg.gz,b1
preboot.subimg.gz,b2
format,sd1
EOF

    # sd1 is the factory_setting partition, already being formatted
    partition_count=2
    for partition in $(grep -v '#' ${EMMC_PART_LIST_FILE} | cut -d'_' -f 1); do
        # factory_setting has nothing to flash and is already being formatted
        [ "${partition}" = "factory" ] && continue
        if [ "${partition}" = "rootfs" ]; then
            # Check if our partitioning is aligned with expected U-Boot env settings
            [ "${UBOOT_BOOT_PARTITION_NUMBER}" != "${partition_count}" ] && bbfatal "U-Boot UBOOT_BOOT_PARTITION_NUMBER (= ${UBOOT_BOOT_PARTITION_NUMBER}) is set to a value different than the rootfs partition (= ${partition_count})"
            subimg_cnt=$(ls ${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}/rootfs_ota_s.subimg.[0-9]* | wc -w)
            for subimg in $(seq 0 $(expr $subimg_cnt - 1)); do
                echo "rootfs_ota_s.subimg.${subimg},sd${partition_count}" >> ${EMMC_IMAGE_LIST_FILE}
            done
        else
            echo "${partition}.subimg.gz,sd${partition_count}" >> ${EMMC_IMAGE_LIST_FILE}
        fi
        partition_count=$(expr $partition_count + 1)
    done
}

python () {
    if d.getVar('PN') == "torizon-docker":
        bb.build.addtask('do_generate_emmc_rawimg', 'do_image_complete', 'do_image_ota_ext4', d)
}

do_generate_emmc_rawimg[depends] += " \
    parted-native:do_populate_sysroot \
    util-linux-native:do_populate_sysroot \
    syna-metadata:do_deploy \
    syna-usb-tool:do_deploy \
    syna-uuu:do_deploy \
    syna-uboot:do_deploy \
    syna-winglet-scs:do_deploy \
"

do_generate_emmc_rawimg() {
    set -e

    IMG="${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.img"

    get_mib_size() {
        size_bytes=$(stat -c%s "$1")
        echo $(expr \( $size_bytes + 1048575 \) / 1048576)
    }

    SIZE_KEY=$(get_mib_size "${DEPLOY_DIR_IMAGE}/key.subimg")
    SIZE_TEE=$(get_mib_size "${DEPLOY_DIR_IMAGE}/tee.subimg")
    SIZE_BL=$(get_mib_size "${DEPLOY_DIR_IMAGE}/bootloader.subimg")
    SIZE_FW=$(get_mib_size "${DEPLOY_DIR_IMAGE}/firmware.subimg")
    SIZE_ROOTFS=$(get_mib_size "${DEPLOY_DIR_IMAGE}/rootfs_ota.subimg")

    SIZE_FACTORY=16

    START_KEY=1
    START_TEE=$(expr $START_KEY + $SIZE_KEY)
    START_BL=$(expr $START_TEE + $SIZE_TEE)
    START_FW=$(expr $START_BL + $SIZE_BL)
    START_FACTORY=$(expr $START_FW + $SIZE_FW)
    START_ROOTFS=$(expr $START_FACTORY + $SIZE_FACTORY)
    DISK_END=$(expr $START_ROOTFS + $SIZE_ROOTFS + 1)

    bbnote "Creating dynamic raw image: $IMG (${DISK_END} MiB)"

    truncate -s ${DISK_END}M "$IMG"
    parted -s "$IMG" mklabel gpt

    parted -s "$IMG" mkpart key_a      ${START_KEY}MiB     ${START_TEE}MiB
    parted -s "$IMG" mkpart tzk_a      ${START_TEE}MiB     ${START_BL}MiB
    parted -s "$IMG" mkpart bl_a       ${START_BL}MiB      ${START_FW}MiB
    parted -s "$IMG" mkpart firmware_a ${START_FW}MiB      ${START_FACTORY}MiB
    parted -s "$IMG" mkpart factory    ${START_FACTORY}MiB ${START_ROOTFS}MiB
    parted -s "$IMG" mkpart rootfs     ${START_ROOTFS}MiB  100%

    dd if="${DEPLOY_DIR_IMAGE}/key.subimg"        of="$IMG" bs=1M seek=$START_KEY     conv=notrunc
    dd if="${DEPLOY_DIR_IMAGE}/tee.subimg"        of="$IMG" bs=1M seek=$START_TEE     conv=notrunc
    dd if="${DEPLOY_DIR_IMAGE}/bootloader.subimg" of="$IMG" bs=1M seek=$START_BL      conv=notrunc
    dd if="${DEPLOY_DIR_IMAGE}/firmware.subimg"   of="$IMG" bs=1M seek=$START_FW      conv=notrunc
    dd if="${DEPLOY_DIR_IMAGE}/rootfs_ota.subimg" of="$IMG" bs=1M seek=$START_ROOTFS  conv=notrunc

    FLASH_DIR="${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}-flash"
    install -d "$FLASH_DIR"
    cp "$IMG" "$FLASH_DIR/emmc.img"

    cp -L -R ${DEPLOY_DIR_IMAGE}/syna-metadata/* "$FLASH_DIR/"
    cp -L -R ${DEPLOY_DIR_IMAGE}/syna-usb-tool/* "$FLASH_DIR/"
    cp -L -R ${DEPLOY_DIR_IMAGE}/syna-uuu/* "$FLASH_DIR/"
    cp -L -R ${DEPLOY_DIR_IMAGE}/syna-uboot/* "$FLASH_DIR/"
    cp -L -R ${DEPLOY_DIR_IMAGE}/syna-winglet-scs/* "$FLASH_DIR/"

    # Create a SYNAIMG-flash.tar.gz file with the raw image, usb-tool and recovery folder inside.
    tar -czf "${DEPLOY_DIR_IMAGE}/${SYNAIMG_DEPLOY_SUBDIR}-flash.tar.gz" \
        -C "${DEPLOY_DIR_IMAGE}" "${SYNAIMG_DEPLOY_SUBDIR}-flash"
}

# Ensure do_image_synaimg runs after do_image_ostree completes since both copy from IMAGE_ROOTFS,
# and if they run simultaneously it can cause FileNotFoundError on temporary files created by rsync/tar.
do_image_synaimg[depends] += "${PN}:do_image_ostree"
