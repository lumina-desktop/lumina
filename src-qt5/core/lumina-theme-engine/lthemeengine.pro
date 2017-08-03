QT       *= core gui widgets x11extras network
!isEqual (QT_MAJOR_VERSION, 5) {
  error("Use Qt 5.4.0 or higher.")
}
include(../../OS-detect.pri)

CONFIG *= c++11

desktop.files=src/lthemeengine/ltheme.desktop
desktop.path=$${L_SHAREDIR}/applications/
                
#Install paths
  PLUGINDIR = $${DISTDIR}$$[QT_INSTALL_PLUGINS]
  DEFINES += LTHEMEENGINE_DATADIR=\\\"$$L_SHAREDIR\\\"

TEMPLATE = subdirs

SUBDIRS += src/lthemeengine-qtplugin src/lthemeengine-style src/lthemeengine

colors.files = colors/*.conf
colors.path = $${L_SHAREDIR}/lthemeengine/colors

INSTALLS += colors
