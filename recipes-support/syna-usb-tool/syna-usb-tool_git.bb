SUMMARY = "Synaptics USB Tool"
DESCRIPTION = "USB tools and drivers needed to flash images to the Synaptics Astra RDK boards"
HOMEPAGE = "https://github.com/synaptics-astra/usb-tool"

LICENSE = "CLOSED"

PV = "1.0.5"
SRC_URI = "git://github.com/synaptics-astra/usb-tool.git;protocol=https;branch=main"
SRCREV = "e5e9a160ffb1755549e73457bda767e641439dc6"

S = "${WORKDIR}/git"

inherit deploy

do_configure[noexec] = "1"
do_compile[noexec] = "1"

SYNAIMG_DEPLOY = "${DEPLOYDIR}/${BPN}"

do_deploy() {
    install -d ${SYNAIMG_DEPLOY}

    mkdir -p ${SYNAIMG_DEPLOY}/bin
    cp -r ${S}/bin/* ${SYNAIMG_DEPLOY}/bin/ || true
    rm -rf ${SYNAIMG_DEPLOY}/bin/*/astra-update 2>/dev/null || true

    cp -r ${S}/astra-usbboot-images/ ${SYNAIMG_DEPLOY}
}

addtask deploy after do_compile

COMPATIBLE_MACHINE = "(sl1680|sl2619|luna-sl1680)"
