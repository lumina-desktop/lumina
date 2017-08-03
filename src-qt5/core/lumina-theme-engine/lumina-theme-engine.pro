include(../../OS-detect.pri)

TEMPLATE = subdirs
SUBDIRS += src/lthemeengine-qtplugin \
		src/lthemeengine-style \
		src/lthemeengine

colors.files = colors/*.conf
colors.path = $${L_SHAREDIR}/lthemeengine/colors

INSTALLS += colors
