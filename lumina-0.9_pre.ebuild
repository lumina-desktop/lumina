# Lumina-DE source code
# Copyright (c) 2016, Ken Moore
# Available under the 3-clause BSD license
# See the LICENSE file for full details

EAPI=5

inherit git-r3 qmake-utils versionator

MY_PV=$(replace_version_separator 3 '-')

DESCRIPTION="Lumina desktop environment"
HOMEPAGE="https://github.com/pcbsd/${PN}"
SRC_URI=""
EGIT_REPO_URI="${HOMEPAGE}"
#EGIT_COMMIT="v${MY_PV}"

LICENSE="BSD"
SLOT="0"
KEYWORDS="~x86 ~amd64"

DEPEND="dev-qt/linguist-tools
    dev-qt/qtconcurrent
	dev-qt/qtcore
	dev-qt/qtmultimedia
	dev-qt/qtnetwork
	dev-qt/qtwidgets
	dev-qt/qtx11extras
	x11-libs/libxcb
	x11-libs/xcb-util
	x11-libs/xcb-util-image
	x11-libs/xcb-util-wm"
RDEPEND="${DEPEND}
	x11-misc/numlockx
	x11-terms/xterm
	x11-wm/fluxbox
	x11-apps/xbacklight
	media-sound/alsa-utils
	sys-power/acpi
	app-admin/sysstat"

src_compile(){

	eqmake5 PREFIX="${ROOT}usr" LIBPREFIX="${ROOT}usr/lib" DESTDIR="${D}" CONFIG+="NO_I18N" || die "eqmake5 failed"
	emake || die "emake failed"
}

src_install(){
	emake install || die "emake install failed"
}
