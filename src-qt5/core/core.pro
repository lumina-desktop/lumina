
include("../OS-detect.pri")

TEMPLATE = subdirs
CONFIG += recursive

SUBDIRS+= lumina-desktop \
	lumina-session \
	lumina-open \
	lumina-info \
	$${PWD}/../../icon-theme \
	lumina-theme-engine
#	lumina-checkpass
#	lumina-desktop-unified

#Also install any special menu scripts
scripts.path = $${L_SHAREDIR}/lumina-desktop/menu-scripts
scripts.files = menu-scripts/*

#Color themes (pre-theme-engine files - not used any more)
#colors.path=$${L_SHAREDIR}/lumina-desktop/colors
#colors.files=colors/*.qss.colors

#Theme templates
themes.path=$${L_SHAREDIR}/lumina-desktop/themes/
themes.files=themes/*.qss.template

#QtQuick plugins
#quickplugins.path=$${L_SHAREDIR}/lumina-desktop/quickplugins/
#quickplugins.files=quickplugins/*

#Mimetype globs
globs.path=$${L_SHAREDIR}/lumina-desktop
globs.files=xtrafiles/globs2

INSTALLS+=scripts themes globs
