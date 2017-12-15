QT       *= core gui widgets x11extras network
!isEqual (QT_MAJOR_VERSION, 5) {
  error("Use Qt 5.4.0 or higher.")
}
include(../../OS-detect.pri)
include(../libLumina/LDesktopUtils.pri)
CONFIG *= c++11

#Install paths
  PLUGINDIR = $${DISTDIR}$$[QT_INSTALL_PLUGINS]
  DEFINES += LTHEMEENGINE_DATADIR=\\\"$$L_SHAREDIR\\\"
