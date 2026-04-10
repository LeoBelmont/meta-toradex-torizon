SUMMARY = "Luna USB boot drivers"
DESCRIPTION = "USB drivers needed to flash images to the Luna board"
HOMEPAGE = "https://github.com/synaptics-astra/usb-tool"

LICENSE = "CLOSED"


SRC_URI = "file://gen3_scs.bin.usb file://gen3_scs_param.bin.usb"

inherit deploy

do_configure[noexec] = "1"
do_compile[noexec] = "1"

SYNAIMG_DEPLOY = "${DEPLOYDIR}/${BPN}"

do_deploy() {
    install -D -m 644 ${WORKDIR}/gen3_scs.bin.usb ${SYNAIMG_DEPLOY}/astra-usbboot-images/sl1680_suboot/gen3_scs.bin.usb
    install -D -m 644 ${WORKDIR}/gen3_scs_param.bin.usb ${SYNAIMG_DEPLOY}/astra-usbboot-images/sl1680_suboot/gen3_scs_param.bin.usb
}

addtask deploy after do_install

COMPATIBLE_MACHINE = "(luna-sl1680)"
