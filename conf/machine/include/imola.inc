# Common Torizon integration for the Arduino UNO Q (machine "imola", QRB2210).
#
# The qcom OTA integration is handled by meta-updater's sota_qcom.bbclass, which
# is auto-inherited (sota.bbclass: SOTA_MACHINE:qcom -> inherit sota_qcom). It
# sets OSTREE_BOOTLOADER=systemd-boot, the qcomflash + ota-esp image types, UKI,
# and OSTree BLS boot-counting rollback — matching the UNO Q's UEFI/systemd-boot
# firmware. We intentionally do NOT override the bootloader, image fstypes or
# kernel args here; doing so conflicts with sota_qcom.
hostname:pn-base-files = "torizon-imola"

# imola.conf adds arduino-firmware to the machine-essential recommends, but it
# pulls arduino-cli (a Go recipe that fetches modules during do_compile) and its
# own do_compile downloads the Arduino core + compiles a sketch — all network
# operations that fail in an offline Yocto build. The MCU firmware/sketch tooling
# is not needed for a minimal bootable image, so drop it. (Re-enable once
# arduino-cli is made offline-buildable via Go module vendoring/premirror.)
MACHINE_ESSENTIAL_EXTRA_RRECOMMENDS:remove = "arduino-firmware"
