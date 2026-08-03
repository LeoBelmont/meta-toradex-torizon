# meta-nxp-mr uses AUTOREV; pin it so the patches apply. See the patch headers.
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRCREV:imx95-navq = "173301e788537094b8322eb06ece75e7a004905e"
SRC_URI:append:imx95-navq = " \
    file://0001-imx95-navqb-add-neutron-NPU-DMA-pool.patch \
    file://0002-imx95-navqb-disable-unpopulated-sja1110-switch.patch \
    file://0003-imx95-navqb-disable-bt-uart.patch \
    file://navq-netfilter.cfg \
"
