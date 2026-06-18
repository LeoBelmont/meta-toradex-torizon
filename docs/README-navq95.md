Common Torizon OS for the NXP NavQ95
====================================
The [NavQ95](https://github.com/rudislabs/navqplus) (NXP Mobile Robotics
NavQ95, i.MX95) is supported as a Common Torizon machine named **`imx95-navqa`**.

It follows the common-machine model: the BSP/kernel/dtb come from NXP's
`meta-imx` plus the [`NXP-Robotics/meta-nxp-mr`](https://github.com/NXP-Robotics/meta-nxp-mr)
Mobile Robotics layer (branch `lf-6.12.20-2.0.0-walnascar-navq95`, which provides
the NavQ machines and the `imx95-navqa.dtb`, and fetches the kernel/u-boot from
the public `NXP-Robotics/linux-imx` and `uboot-imx`), and the `common-torizon`
distro is layered on top. Because it is the same i.MX95 SoC, u-boot bootloader
and wic layout as the Verdin i.MX95 EVK, the Torizon integration
([conf/machine/include/imx95-navqa.inc](../conf/machine/include/imx95-navqa.inc))
mirrors the Verdin one.

> Use `NXP-Robotics/meta-nxp-mr`, not the `rudislabs/meta-nxp-mr` fork — the
> latter points the i.MX95 kernel/u-boot at *private* NXP-Robotics repos.

This is built on the **walnascar** Yocto line — the line the NavQ95 BSP
(`meta-nxp-mr`, `lf-6.12.20-2.0.0-walnascar-navq95`) ships on, which matches
Torizon's walnascar branch.

Setup
=====
The integration manifest [docs/navq95-integration.xml](./navq95-integration.xml)
is self-contained: it pins the same i.MX95 walnascar layer set Toradex uses
(base + NXP BSP + Torizon, including the walnascar-pinned `meta-updater`), adds
`meta-nxp-mr`, and pulls `meta-toradex-torizon` (with the `imx95-navqa` machine)
from this fork. So you can `repo init` straight from the fork — no
`toradex-manifest` clone needed:

```bash
$ mkdir navq95; cd navq95
$ repo init -u https://github.com/LeoBelmont/meta-toradex-torizon \
       -b walnascar-navq95 -m docs/navq95-integration.xml
$ repo sync -j"$(nproc)"
```
> The `imx95-navqa` machine lives only in this fork's `meta-toradex-torizon`
> (branch `walnascar-navq95`). Every other layer is an unmodified dependency
> pulled from its canonical home. Once the machine lands in
> `torizon/meta-toradex-torizon`'s walnascar branch, repoint that project.

Build
=====
| Board                 | MACHINE       | Status       |
|-----------------------|---------------|--------------|
| NavQ95 rev A          | imx95-navqa   | Experimental |
| NavQ95 rev B ("V2")   | imx95-navqb   | Experimental |

> [!IMPORTANT]
> **Pick the MACHINE that matches your board revision.** The variants differ in
> their System Manager firmware config (`mr-navq95a` vs `mr-navq95b`), which sets
> up the SoC clocks/pinmux. Building the wrong variant boots, but the SM
> mis-configures peripherals (e.g. the CAN clock) and drivers fault. The board
> reports its revision in the kernel log (`Hardware name: NXP X-MR-NAVQ95A/B`) and
> on the serial-download SPL filename (`MR-NAVQ95A/B-…`). The "V2" board is rev B.

`repo sync` links our script as `torizon-setup-environment` (from
`meta-toradex-torizon`), NXP's `setup-environment` (from `fsl-community-bsp-base`)
and the NavQ `imx-setup-release.sh` (from `meta-nxp-mr`) to the project root.

> [!IMPORTANT]
> Source **`torizon-setup-environment`**, not `setup-environment`. The latter is
> NXP's script, which `imx-setup-release.sh` re-sources internally — naming ours
> differently avoids the clash.

Our script dispatches to `setup-environment-imx`, which sources
`imx-setup-release.sh` (it knows the NavQ machines, and adds meta-arm,
meta-virtualization, meta-security, …), then appends the Torizon layers and sets
`DISTRO=common-torizon`:
```bash
$ MACHINE=imx95-navqb . torizon-setup-environment    # or imx95-navqa for rev A
```
Then build a minimal image:
```bash
$ bitbake torizon-minimal
```
Artifacts (including the `.wic`) land in `build/deploy/images/<MACHINE>`.

The NavQ u-boot boots via bootstd (`bootflow scan`), which doesn't source
Torizon's `boot.scr`; this layer overrides `CONFIG_BOOTCOMMAND`
([dynamic-layers/navq/recipes-bsp/u-boot](../dynamic-layers/navq/recipes-bsp/u-boot/))
so the board auto-boots the OSTree image from eMMC/SD.

Cortex-M7 firmware
==================
The NavQ95's Cortex-M7 runs an MCUboot firmware that NXP embeds into the boot
container (`flash.bin`). This is enabled by default for `imx95-navqa`: the M7
firmware (`zephyr-mcuboot`) is built in a separate Zephyr multiconfig and baked
into `imx-boot`. The wiring lives entirely in this layer, so no extra steps are
needed — just build `torizon-minimal`.

How it works:
- [conf/multiconfig/imx95-navq-m7.conf](../conf/multiconfig/imx95-navq-m7.conf)
  defines the M7 (`MACHINE=imx95-navqa-m7`, `DISTRO=zephyr`) multiconfig, enabled
  via `BBMULTICONFIG` in [imx95-navqa.inc](../conf/machine/include/imx95-navqa.inc).
- [dynamic-layers/navq-m7/…/imx-boot_%.bbappend](../dynamic-layers/navq-m7/recipes-bsp/imx-mkimage/imx-boot_%25.bbappend)
  retargets `meta-nxp-mr`'s `imx-boot` multiconfig dependency from the *desktop*
  multiconfig to our default one (NXP only wires the M7 up for their
  `imx95-navqadesktop`/`imx95-navqbdesktop` machines).

> The Zephyr M7 build pulls the `meta-zephyr` toolchain; the firmware source is
> `zephyr-mcuboot` from `meta-nxp-mr` (board overlay `mr_navq95a`). This is the
> M7 *bootloader*, embedded at boot — not a Linux-`remoteproc` payload.

To build a **plain Linux image without the M7 firmware**, comment out the
`BBMULTICONFIG` line in `imx95-navqa.inc` and re-mask the `imx-boot` bbappend
(`BBMASK += "/meta-nxp-mr/recipes-bsp/imx-mkimage/"`).

Flash the Device
================
The NavQ95 uses the standard i.MX95 boot flow, so flashing works like the
Verdin i.MX95 EVK — see the [NXP README](./README-nxp.md) for `uuu`/SD-card
instructions, substituting the `imx95-navqa` artifacts.
