SUMMARY = "U-boot binaries for the Synaptics Astra RDK and Luna boards"
DESCRIPTION = "Fastboot/UUU enabled Synaptics USB u-boot binaries"

LICENSE = "CLOSED"

SRC_URI = "\
    file://gen3_uboot.bin.usb \
    file://gen3_uboot.bin.usb.header \
    file://uEnv.txt \
"

inherit deploy

do_configure[noexec] = "1"
do_compile[noexec] = "1"

SYNAIMG_DEPLOY = "${DEPLOYDIR}/${BPN}"

do_deploy() {
    install -D -m 644 ${WORKDIR}/gen3_uboot.bin.usb ${SYNAIMG_DEPLOY}/astra-usbboot-images/sl1680_suboot/gen3_uboot.bin.usb
    install -D -m 644 ${WORKDIR}/gen3_uboot.bin.usb.header ${SYNAIMG_DEPLOY}/astra-usbboot-images/sl1680_suboot/gen3_uboot.bin.usb.header
    install -D -m 644 ${WORKDIR}/uEnv.txt ${SYNAIMG_DEPLOY}/astra-usbboot-images/sl1680_suboot/uEnv.txt
}

addtask deploy after do_install

COMPATIBLE_MACHINE = "(sl1680|sl2619|luna-sl1680)"
