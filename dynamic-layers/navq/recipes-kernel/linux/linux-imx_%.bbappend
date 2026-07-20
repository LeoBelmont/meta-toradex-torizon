# Pin the kernel to ensure patch will always work

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRCREV:imx95-navq = "80974408d079cf98d7ed7797880da974c3c576d5"
SRC_URI:append:imx95-navq = " file://0001-imx95-navqb-add-neutron-NPU-DMA-pool.patch"
