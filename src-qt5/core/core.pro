
include("../OS-detect.pri")

TEMPLATE = subdirs
CONFIG += recursive

# lumina-checkpass is needed for 1.6.2, since code replacing
# xscreensaver is enabled
SUBDIRS+= lumina-desktop \
	lumina-session \
	lumina-open \
	lumina-info \
	lumina-pingcursor \
	$${PWD}/../../icon-theme \
	lumina-theme-engine \
	lumina-checkpass
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

#FluxBox Config
fcfg.path=$${L_SHAREDIR}/fluxbox/style/lumina-dark/theme.cfg
fcfg.files=extrafiles/theme.cfg

#QtQuick plugins
#quickplugins.path=$${L_SHAREDIR}/lumina-desktop/quickplugins/
#quickplugins.files=quickplugins/*

#Mimetype globs
globs.path=$${L_SHAREDIR}/lumina-desktop
globs.files=xtrafiles/globs2

INSTALLS+=scripts themes globs fcfg
