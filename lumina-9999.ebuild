# Lumina-DE source code
# Copyright (c) 2016, Ken Moore
# Available under the 3-clause BSD license
# See the LICENSE file for full details

EAPI=6

inherit git-r3 qmake-utils versionator

DESCRIPTION="Lumina desktop environment"
HOMEPAGE="https://github.com/pcbsd/${PN}"
SRC_URI=""
EGIT_REPO_URI="${HOMEPAGE}"

LICENSE="BSD"
SLOT="0"
KEYWORDS="~x86 ~amd64"

DEPEND="dev-qt/linguist-tools:5
    dev-qt/qtconcurrent:5
	dev-qt/qtcore:5
	dev-qt/qtmultimedia:5
	dev-qt/qtnetwork:5
	dev-qt/qtwidgets:5
	dev-qt/qtx11extras:5
	x11-libs/libxcb
	x11-libs/xcb-util
	x11-libs/xcb-util-image
	x11-libs/xcb-util-wm"
RDEPEND="${DEPEND}
	x11-misc/numlockx
	x11-wm/fluxbox
	x11-apps/xbacklight
	media-sound/alsa-utils
	sys-power/acpi
	app-admin/sysstat"

src_compile(){

	eqmake5 PREFIX="${ROOT}usr" LIBPREFIX="${ROOT}usr/lib" DESTDIR="${D}" CONFIG+="NO_I18N"
	emake
}
