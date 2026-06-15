Common Torizon OS for the Arduino UNO Q (imola)
===============================================

Build **Common Torizon OS** for the **Arduino UNO Q** (Yocto machine **`imola`**,
Qualcomm **QRB2210 / QCM2290** SoC).

| Board         | MACHINE | SoC               | Status       |
|---------------|---------|-------------------|--------------|
| Arduino UNO Q | imola   | QRB2210 / QCM2290 | Experimental |

> [!IMPORTANT]
> The Qualcomm BSP for the UNO Q exists only on the **`wrynose`** Yocto line, so
> this is built against `wrynose` using the **`master`** branch of this layer
> (which targets wrynose) plus Qualcomm's wrynose BSP.

How it boots / flashes
======================
The UNO Q boots ABL → UEFI (EDK2) → **systemd-boot**. meta-updater's
`sota_qcom.bbclass` is auto-inherited (`SOTA_MACHINE:qcom`) and drives the OTA
integration: `OSTREE_BOOTLOADER=systemd-boot`, `qcomflash` + `ota-esp` images,
UKI, and OSTree BLS boot-counting for rollback. So the `imola` machine include
(`conf/machine/include/imola.inc`) adds almost nothing — it intentionally does
not override the bootloader/image/kernel-args.

Flash the `<image>.qcomflash/` directory (under `deploy/images/imola/`) with
`qdl` over USB-C in EDL (9008) mode. First-boot debugging needs a UART adapter on
the `ttyMSM0` debug pins (the USB-C exposes no serial console).

Quick start (repo manifest)
===========================
A custom manifest, [`arduino-imola.xml`](./arduino-imola.xml), pins the whole
layer set (Qualcomm wrynose BSP + meta-arduino + this layer + meta-poky). Put it
in a small git repo (the manifest repo) and:

```bash
mkdir imola && cd imola
repo init -u <git-repo-holding-arduino-imola.xml> -m arduino-imola.xml
repo sync -j"$(nproc)"
MACHINE=imola . setup-environment       # dispatches to setup-environment-qcom
bitbake torizon-minimal
```

`setup-environment` (symlinked from this layer) detects `imola` and runs
`scripts/lib/setup-devices/setup-environment-qcom`, which writes `bblayers.conf`
+ `local.conf`, drops the unavailable `qcom-3rdparty` layer dep, and sets the
qcom mirrors/`OEROOT`/`BBMASK`s. `uno-q.conf` is shipped by this layer
(`conf/machine/uno-q.conf`). So after `repo sync` it's two commands to a build.

Manual setup
============
If you prefer to assemble the tree by hand instead of using the manifest:

1. Sync the Qualcomm BSP:
   ```bash
   mkdir imola && cd imola
   repo init -u https://github.com/qualcomm-linux/qcom-manifest -b qcom-linux-wrynose -m qli-2.0-rc3.xml
   repo sync -j"$(nproc)"
   ```
   Provides `oe-core`, `meta-openembedded`, `meta-qcom`, `meta-updater` (sota),
   `meta-virtualization`, `bitbake` — all wrynose, at the top level.

2. Clone the Arduino, Torizon and poky layers:
   ```bash
   git clone https://github.com/arduino/meta-arduino                       # meta-arduino-qcom is on the default 'master'
   git clone https://github.com/torizon/meta-toradex-torizon -b master     # master targets wrynose
   git clone -b wrynose https://git.yoctoproject.org/git/meta-yocto          # provides meta-poky on a real wrynose branch
   ```
   (The combined `poky` repo has no usable wrynose branch — its master is a
   stub — but `meta-poky` ships standalone in `meta-yocto`, which does.)

3. Small tree adjustments (these live outside this layer):
   ```bash
   # meta-qcom-3rdparty has no wrynose branch; drop the dependency
   sed -i 's/ qcom-3rdparty//' meta-arduino/meta-arduino-qcom/conf/layer.conf
   ```

4. Provide `uno-q.conf` (required by `imola.conf`; not shipped upstream — model
   it on meta-qcom's QRB2210 `rb1-core-kit`):
   ```bash
   cat > meta-arduino/meta-arduino-qcom/conf/machine/uno-q.conf <<'EOF'
   #@TYPE: Machine (base)
   #@NAME: Arduino UNO Q base (QRB2210 / QCM2290)
   require conf/machine/include/qcom-qcm2290.inc
   MACHINE_FEATURES = "efi usbhost usbgadget alsa wifi bluetooth"
   PREFERRED_PROVIDER_virtual/bootloader ?= "u-boot"
   UBOOT_CONFIG = "qrb2210-rb1"
   QCOM_DTB_DEFAULT ?= "qrb2210-rb1"
   KERNEL_DEVICETREE ?= " qcom/qrb2210-rb1.dtb "
   MACHINE_ESSENTIAL_EXTRA_RRECOMMENDS += " packagegroup-rb1-firmware packagegroup-rb1-hexagon-dsp-binaries qbootctl"
   QCOM_BOOT_FILES_SUBDIR = "qrb2210"
   QCOM_PARTITION_FILES_SUBDIR ?= "partitions/qrb2210-rb1/emmc"
   QCOM_BOOT_FIRMWARE = "firmware-qcom-boot-qrb2210-rb1"
   EOF
   ```

Build
=====
```bash
source oe-core/oe-init-build-env build
```

`conf/bblayers.conf` (adjust `<ROOT>`):
```
POKY_BBLAYERS_CONF_VERSION = "2"
BBPATH = "${TOPDIR}"
BBFILES ?= ""
BBLAYERS ?= " \
  <ROOT>/meta-yocto/meta-poky \
  <ROOT>/oe-core/meta \
  <ROOT>/meta-openembedded/meta-oe \
  <ROOT>/meta-openembedded/meta-python \
  <ROOT>/meta-openembedded/meta-networking \
  <ROOT>/meta-openembedded/meta-filesystems \
  <ROOT>/meta-virtualization \
  <ROOT>/meta-updater \
  <ROOT>/meta-qcom \
  <ROOT>/meta-arduino/meta-arduino-common \
  <ROOT>/meta-arduino/meta-arduino-qcom \
  <ROOT>/meta-toradex-torizon \
"
```

Append to `conf/local.conf`:
```
MACHINE = "imola"
DISTRO = "common-torizon"

# qcom sources come from the CodeLinaro mirrors
MIRRORS:append = " \
    git://github.com git://git.codelinaro.org/clo/yocto-mirrors/github/ \
    git://.*/ git://git.codelinaro.org/clo/yocto-mirrors/ \
    https://.*/.*/ https://artifacts.codelinaro.org/codelinaro-le/ \
"
SKIP_META_QCOM_SANITY_CHECK = "1"
INHERIT:remove = "uninative"

# ostree_layer_revision_info.bbclass uses ${OEROOT}, which is only set by
# Toradex's setup-environment; set it here when building via oe-init-build-env.
OEROOT = "<ROOT>"

# Recipes not used by / not buildable for this minimal imola build
BBMASK += "linux-yocto-fitimage"
BBMASK += "linux-arduino-rt"
BBMASK += "linux-arduino_%.bbappend"
```

Build:
```bash
bitbake torizon-minimal
```
Artifacts land in `build/deploy/images/imola/` (including `<image>.qcomflash/`).

Notes
=====
- The wrynose port lives on this layer's `master` branch; this machine adds only
  `conf/machine/include/imola.inc` (defers to `sota_qcom`, drops the build-time
  `arduino-firmware`), the `meta-arduino-qcom` entry in `conf/layer.conf`'s
  `BBFILES_DYNAMIC`, and a `linux-qcom-next` bbappend for the OSTree initramfs.
- `uno-q.conf`, the `qcom-3rdparty` tweak, and bblayers/local.conf
  live outside this layer and must be applied as above.
- `arduino-firmware` (STM32 MCU firmware/sketch tooling) is excluded because
  `arduino-cli` fetches Go modules during `do_compile`, which fails in an offline
  build; it's not needed to boot. Re-enable once `arduino-cli` is offline-buildable.
