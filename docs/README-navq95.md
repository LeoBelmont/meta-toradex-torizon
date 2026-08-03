Common Torizon OS for the NXP NavQ95
====================================
The [NavQ95](https://github.com/rudislabs/navqplus) (NXP Mobile Robotics
NavQ95, i.MX95) is supported as Common Torizon machines **`imx95-navqa`** (rev A)
and **`imx95-navqb`** (rev B / "V2").

It follows the common-machine model: the BSP/kernel/dtb come from NXP's
`meta-imx` plus the [`NXP-Robotics/meta-nxp-mr`](https://github.com/NXP-Robotics/meta-nxp-mr)
Mobile Robotics layer (branch `lf-6.18.2-2.0.0-wrynose-navq95`, which provides
the NavQ machines and dtbs and fetches the kernel/u-boot from the public
`NXP-Robotics/linux-imx` and `uboot-imx`), and the `common-torizon` distro is
layered on top. Because it is the same i.MX95 SoC, u-boot bootloader and wic
layout as the Verdin i.MX95 EVK, the Torizon integration
([conf/machine/include/imx95-navq.inc](../conf/machine/include/imx95-navq.inc))
mirrors the Verdin one.

> Use `NXP-Robotics/meta-nxp-mr`, not the `rudislabs/meta-nxp-mr` fork — the
> latter points the i.MX95 kernel/u-boot at *private* NXP-Robotics repos.

This is built on the **wrynose** Yocto line (Yocto 6.0, Linux 6.18) — matching
the NavQ95 BSP (`meta-nxp-mr`, `lf-6.18.2-2.0.0-wrynose-navq95`, on NXP's
`rel_imx_6.18.20_2.0.0` i.MX BSP) and Torizon's wrynose (`master`) branch.

> [!NOTE]
> This wrynose port was migrated from the `walnascar-navq95` branch. Verified on
> hardware: boot, eth0, Wi-Fi driver, Docker, the Cortex-M7, and Neutron NPU
> inference. Bluetooth is not supported (see "Migration status" below).

Setup
=====
The integration manifest lives in the `toradex-manifest` fork at
[`common-torizon/nxp/navq95.xml`](https://github.com/LeoBelmont/toradex-manifest/blob/wrynose-navq95/common-torizon/nxp/navq95.xml).
It pins the wrynose i.MX95 layer set (base + NXP BSP + Torizon), adds
`meta-nxp-mr`, and pulls `meta-toradex-torizon` (with the NavQ machines) from
this fork:

```bash
$ mkdir navq95; cd navq95
$ repo init -u https://github.com/LeoBelmont/toradex-manifest \
       -b wrynose-navq95 -m common-torizon/nxp/navq95.xml
$ repo sync -j"$(nproc)"
```
> The NavQ machines live only in this fork's `meta-toradex-torizon`
> (branch `wrynose-navq95`). Every other layer is an unmodified dependency
> pulled from its canonical home. Once the machines land in
> `torizon/meta-toradex-torizon`'s master branch, repoint that project.

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

```bash
$ MACHINE=imx95-navqb . torizon-setup-environment    # or imx95-navqa for rev A
$ bitbake torizon-minimal
```
Artifacts (including the `.wic`) land in `build/deploy/images/<MACHINE>`.

The NavQ u-boot boots via bootstd (`bootflow scan`), which doesn't source
Torizon's `boot.scr`. This layer adds a Kconfig fragment
([dynamic-layers/navq/recipes-bsp/u-boot/files/navq-bootcommand.cfg](../dynamic-layers/navq/recipes-bsp/u-boot/files/navq-bootcommand.cfg))
that overrides `CONFIG_BOOTCOMMAND` to source `/boot.scr` from the OTA root
(eMMC then SD), the same mechanism the Verdin i.MX95 EVK uses.

Cortex-M7 firmware
==================
The NavQ95's Cortex-M7 runs an MCUboot firmware that NXP embeds into the boot
container (`flash.bin`), enabled by default: `zephyr-mcuboot` is built in a
separate Zephyr multiconfig and baked into `imx-boot`.

- [conf/multiconfig/imx95-navqa-m7.conf](../conf/multiconfig/imx95-navqa-m7.conf)
  defines the M7 (`MACHINE=imx95-navqa-m7`, `DISTRO=zephyr`) multiconfig, enabled
  via `BBMULTICONFIG` in [imx95-navqa.inc](../conf/machine/include/imx95-navqa.inc).
- [dynamic-layers/navq/…/imx-boot_%.bbappend](../dynamic-layers/navq/recipes-bsp/imx-mkimage/imx-boot_%25.bbappend)
  retargets `meta-nxp-mr`'s `imx-boot` multiconfig dependency from the *desktop*
  multiconfig to ours (NXP only wires the M7 up for their `imx95-navq{a,b}desktop`
  machines).

To build a **plain Linux image without the M7 firmware**, comment out the
`BBMULTICONFIG` line in `imx95-navqa.inc` and re-mask the `imx-boot` bbappend
(`BBMASK += "/meta-nxp-mr/recipes-bsp/imx-mkimage/"`).

Migration status (walnascar → wrynose)
=======================================
This wrynose port relies on the upstream `meta-nxp-mr` / `meta-imx` wrynose
defaults. Kernel patches and config live in
`dynamic-layers/navq/recipes-kernel/linux/`, applied against pinned kernel tip
`173301e7`:

- `0001` — Neutron NPU reserved-memory DMA pool. The 6.18 `imx95.dtsi` neutron node
  still has no `memory-region`, and the NavQ dts never applies NXP's EVK-only
  `imx95-19x19-evk-neutron.dtso`, so the pool is still required. **NPU inference
  verified working.**
- `0002` — disable the unpopulated SJA1110 switch and its `enetc_port2` CPU port.
  The switch reads an all-ones device ID (not fitted) and meta-nxp-mr carries no
  SJA1110 firmware on wrynose. `enetc_port2` is a `fixed-link`, so it advertised a
  permanent fake carrier and made NetworkManager retry DHCP forever. The board has
  a single RJ45, on `enetc_port0`.
- `0003` — disable the BT UART. See Bluetooth below.
- `navq-netfilter.cfg` — `CONFIG_IP_NF_RAW` / `CONFIG_IP6_NF_RAW`. Docker's DIRECT
  ACCESS FILTERING rule needs the iptables `raw` table; without it every container
  fails with `can't initialize iptables table 'raw'`. Also fixes VS Code debugging.

The walnascar IRQ-completion backport into `drivers/staging/neutron` was **dropped
as obsolete** — the 6.18 driver already carries the stock IRQ path
(`NEUTRON_USE_IRQ_MODE`, `neutron_irq_enable()`, the mailbox `APPSTATUS` W1C clear).

The **Neutron firmware** (`NeutronFirmware.elf`) is loaded by the host kernel via
remoteproc, so it must be in the host rootfs — a container shipping its own delegate
is not enough. Torizon did not install it for any i.MX95 machine, which failed as
`remoteproc0: request_firmware failed: -110`; the `neutron` package is now added via
`CORE_IMAGE_BASE_INSTALL` in `torizon-base.inc`. The walnascar firmware version pin
is not needed: wrynose's `neutron` (`lf-6.18.20_2.0.0`) matches the delegate. Re-pin
only if a `Microcode version mismatch` appears.

**Bluetooth is not supported.** The firmware is present (`uart8987_bt.bin`) but the
chip never answers `btnxpuart`'s bootloader handshake (`FW Download Timeout.
offset: 0`), a hardcoded 60 s wait with no module parameter, so `0003` disables the
node. Most likely NXP's out-of-tree `moal` Wi-Fi driver brings the combo part up
over SDIO (`sduart8987_combo.bin`) before `btnxpuart` probes, leaving the UART side
past its bootloader; the module reset is also rejected (`reset-gpio code does not
support GPIO flags 7 for GPIO 16`, from `usdhc3_pwrseq`). To pursue it: point `moal`
at the Wi-Fi-only `sd8987_wlan.bin` so `btnxpuart` can own the BT firmware, or keep
the combo firmware and attach from userspace (`btattach -P h4`). Either way the
image has no BT userspace — `bluetooth` is not in `DISTRO_FEATURES`.
- **System Manager pin** (`imx-system-manager_%.bbappend`) — dropped; the SM now
  builds from the `imx95-navq-lf-6.18.y-26Q2` branch at `AUTOREV`. Re-pin a tip
  for reproducibility once verified.
- **Build workarounds** — `INITRAMFS_EXTRA_KMODS` clear and
  `IMAGE_LOG_CHECK_EXCLUDES` ctrl-alt-del are **still needed** on wrynose and were
  re-added to `imx95-navq.inc`. The first is because the shared `mx95-nxp-bsp` splash
  kmod list is the Verdin i.MX95 display chain, which NavQ's kernel does not build as
  modules (NavQ also has no video output on the base board — display is only via
  `dsicsi_b2b` add-on overlays). The `python3-flit-core` mask is **not** needed.
- **aktualizr-torizon SRCREV bump** — dropped; the Boost fix is handled on
  Torizon `master`.

Flash the Device
================
Follow NXP's own NavQ95 flashing guide in
[NXP-Robotics/imx-manifest-navq95](https://github.com/NXP-Robotics/imx-manifest-navq95),
substituting the `imx95-navqa`/`imx95-navqb` artifacts from
`build/deploy/images/<MACHINE>`. The board uses the standard i.MX95 boot flow, so
the `uuu`/SD-card steps in the [NXP README](./README-nxp.md) apply too.
