SUMMARY = "Synaptics UUU Flashing Tool"
DESCRIPTION = "UUU binary for flashing Synaptics Astra boards via USB"
HOMEPAGE = "https://github.com/nxp-imx/mfgtools"

LICENSE = "CLOSED"

SRC_URI = "file://uuu \
           file://uuu.auto \
           file://uuu.exe \
"

S = "${WORKDIR}"

inherit deploy

do_configure[noexec] = "1"
do_compile[noexec] = "1"

SYNAIMG_DEPLOY = "${DEPLOYDIR}/${BPN}"

do_deploy() {
    install -d ${SYNAIMG_DEPLOY}/bin/linux/x86_64
    install -d ${SYNAIMG_DEPLOY}/bin/windows/x86_64

    install -m 0755 ${S}/uuu ${SYNAIMG_DEPLOY}/bin/linux/x86_64/

    if [ -f ${S}/uuu.auto ]; then
        install -m 0755 ${S}/uuu.auto ${SYNAIMG_DEPLOY}/bin/linux/x86_64/
    fi

    if [ -f ${S}/uuu.exe ]; then
        install -m 0755 ${S}/uuu.exe ${SYNAIMG_DEPLOY}/bin/windows/x86_64/
    fi
}

addtask deploy after do_compile

COMPATIBLE_MACHINE = "(sl1680|sl2619|luna-sl1680)"
