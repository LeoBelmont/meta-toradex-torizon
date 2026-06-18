FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI += "file://torizon-bootcmd.cfg"

uboot_torizon_bootcmd() {
    local cfgdir="${1}"
    [ -f "${cfgdir}/.config" ] || return 0
    sed -i '/^CONFIG_BOOTCOMMAND=/d' "${cfgdir}/.config"
    cat "${UNPACKDIR}/torizon-bootcmd.cfg" >> "${cfgdir}/.config"
    oe_runmake -C "${S}" O="${cfgdir}" olddefconfig
}

do_configure:append() {
    if [ -n "${UBOOT_CONFIG}" ]; then
        for config in ${UBOOT_MACHINE}; do
            uboot_torizon_bootcmd "${B}/${config}"
        done
    else
        uboot_torizon_bootcmd "${B}"
    fi
}
