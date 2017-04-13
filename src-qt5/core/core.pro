
include("../OS-detect.pri")

TEMPLATE = subdirs
CONFIG += recursive

SUBDIRS+= lumina-desktop \
	lumina-session \
	lumina-open \
	lumina-info \
	$${PWD}/../../icon-theme
#	lumina-wm-INCOMPLETE \
#	lumina-checkpass

#Also install any special menu scripts
scripts.path = $${L_SHAREDIR}/lumina-desktop/menu-scripts
scripts.files = menu-scripts/*

#Color themes
colors.path=$${L_SHAREDIR}/lumina-desktop/colors
colors.files=colors/*.qss.colors

#Theme templates
themes.path=$${L_SHAREDIR}/lumina-desktop/themes/
themes.files=themes/*.qss.template

#QtQuick plugins
#quickplugins.path=$${L_SHAREDIR}/lumina-desktop/quickplugins/
#quickplugins.files=quickplugins/*

#Mimetype globs
globs.path=$${L_SHAREDIR}/lumina-desktop
globs.files=xtrafiles/globs2

INSTALLS+=scripts colors themes globs
