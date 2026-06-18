# Retarget meta-nxp-mr's imx-boot M7-firmware mcdepends from the NavQ desktop multiconfig
do_configure[mcdepends] = "mc::${NAVQ_M7_MC}:zephyr-mcuboot:do_deploy"
