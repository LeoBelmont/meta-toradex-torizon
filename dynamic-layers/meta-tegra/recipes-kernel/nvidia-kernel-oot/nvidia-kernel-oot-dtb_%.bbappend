# As of L4T r39.2 the tegra-devicetree class builds the DTBs in this recipe and
# deploys them to ${DEPLOYDIR}/devicetree/ (previously they came from a staged
# dependency, hence the old ${STAGING_DIR_HOST}/boot/devicetree path). Torizon
# expects the DTBs at the deploy root, so copy them up after the class deploy and
# verify the machine's KERNEL_DEVICETREE is present.
do_deploy:append() {
    for dtb in ${KERNEL_DEVICETREE}; do
        dtbf="${DEPLOYDIR}/devicetree/$dtb"
        if [ ! -f "$dtbf" ]; then
            bbfatal "Not found: $dtbf"
        fi
    done
    install -m 0644 ${DEPLOYDIR}/devicetree/*.dtb ${DEPLOYDIR}/
}
