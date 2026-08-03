# Retarget meta-nxp-mr's mcdepends from its desktop multiconfig to ours.
do_configure[mcdepends] = "mc::${NAVQ_M7_MC}:zephyr-mcuboot:do_deploy"
