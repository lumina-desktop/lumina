#Some conf to redirect intermediate stuff in separate dirs
UI_DIR=./.build/ui/
MOC_DIR=./.build/moc/
OBJECTS_DIR=./.build/obj
RCC_DIR=./.build/rcc

QMAKE_DISTCLEAN += -r .build

CONFIG += c++11

#Install paths
unix {
  isEmpty(PREFIX) {
    PREFIX = /usr/local/
  }
  isEmpty(PLUGINDIR) {
    PLUGINDIR = $$[QT_INSTALL_PLUGINS]
  }

  BINDIR = $$PREFIX/bin
  DATADIR = $$PREFIX/share

  DEFINES += LTHEMEENGINE_DATADIR=\\\"$$DATADIR\\\"
}
