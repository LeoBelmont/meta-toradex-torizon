require recipes-kernel/linux/linux-torizon.inc

# torizon.cfg now applies via torizon.scc; NavQ is missing from that list.
SRC_URI:append:imx95-navq = " file://torizon.cfg"
