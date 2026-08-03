# Make bootstd u-boot source Torizon's /boot.scr.
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append:imx95-navq = " file://navq-bootcommand.cfg"
