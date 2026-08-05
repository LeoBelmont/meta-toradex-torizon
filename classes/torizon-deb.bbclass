# Produce a .deb from a recipe, for installing into a Debian container.
#
# Torizon ships vendor userspace to containers as Debian packages.  OE's
# package_deb backend gets most of the way there: it writes a real .deb with the
# payload, Description, Section, Maintainer, Homepage and maintainer scripts.
# What it cannot do is compute Debian dependencies -- OE resolves SONAMEs to OE
# package names (libdrm, not libdrm2), and versioned dependencies need Debian's
# shlibs data.  That requires a Debian sysroot, so it happens afterwards, in
# scripts/debianize-debs.
#
#   inherit torizon-deb                     (in a .bbappend)
#   bitbake -c package_write_deb imx-dsp
#   ls tmp/deploy/deb/*/
#   scripts/debianize-debs -o out tmp/deploy/deb/imx8mp/imx-dsp_*.deb
#
# Set in the .bbappend only what the recipe cannot imply:
#
#   SUMMARY      one-line synopsis; bitbake's default is "${PN} version ${PV}",
#                which is useless as a Debian synopsis, so always set it
#   DESCRIPTION  the long description; package_deb indents and wraps it
#   PKG:${PN}    Debian binary package name, if it differs from the OE one
#
# Do NOT bother setting RDEPENDS in Debian names: debianize-debs replaces
# Depends wholesale from dpkg-shlibdeps.  Keep RDEPENDS correct for OE.
#
# Machine-specific recipes still need a decision this class cannot make.  A
# container package serves a whole SoC family, but the recipe builds for one
# MACHINE -- imx-dsp's do_install deletes every hifi4_*.bin except the current
# machine's.  Fix that with a do_install:append in the .bbappend.

inherit package_deb

# If bitbake complains that do_package_write_deb does not exist for a
# dependency, drop the inherit above and instead put this in local.conf:
#
#   PACKAGE_CLASSES:append = " package_deb"
#
# The first entry in PACKAGE_CLASSES still decides IMAGE_PKGTYPE, so appending
# leaves image construction untouched and only adds deb output.

DEB_REVISION ?= "1+toradex1"

# package_deb writes "Version: ${PKGV}-${PKGR}".  PR is r0, which is not the
# Toradex feed convention, so override the packaging revision only.  PV is
# untouched, so sstate and recipe versioning are unaffected.
PKGR = "${DEB_REVISION}"

MAINTAINER ?= "Toradex Packaging Team <debian-pkg-team@toradex.com>"
PRIORITY ?= "optional"

# Vendor userspace is redistributable but not free.  debianize-debs appends the
# /libs or /libdevel component per package.
SECTION ?= "${@'non-free' if (d.getVar('LICENSE') or '').upper() in ('CLOSED', 'PROPRIETARY') else 'misc'}"
