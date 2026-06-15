require recipes-kernel/linux/linux-torizon.inc

# The vendor qcom kernel recipe sets its own INITRAMFS_IMAGE; point it back to
# the Torizon OSTree initramfs so the image boots into the OTA sysroot.
INITRAMFS_IMAGE = "initramfs-ostree-torizon-image"
