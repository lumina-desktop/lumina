QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras network

QMAKE_DISTCLEAN += -r .build

CONFIG += c++11

desktop.files=src/lthemeengine/ltheme.desktop
desktop.path=$${L_SHAREDIR}/applications/
                
#Install paths
unix {
  isEmpty(PREFIX) {
    PREFIX = /usr
  }
  isEmpty(PLUGINDIR) {
    PLUGINDIR = $$[QT_INSTALL_PLUGINS]
  }

  BINDIR = $$PREFIX/bin
  DATADIR = $$PREFIX/share

  DEFINES += LTHEMEENGINE_DATADIR=\\\"$$DATADIR\\\"
}

TEMPLATE = subdirs

SUBDIRS += src/lthemeengine-qtplugin src/lthemeengine-style src/lthemeengine

unix:exists($$[QT_INSTALL_BINS]/lrelease){
LRELEASE_EXECUTABLE = $$[QT_INSTALL_BINS]/lrelease
}

unix:exists($$[QT_INSTALL_BINS]/lrelease-qt5){
LRELEASE_EXECUTABLE = $$[QT_INSTALL_BINS]/lrelease-qt5
}


isEmpty(LRELEASE_EXECUTABLE){
error(Could not find lrelease executable)
}
else {
message(Found lrelease executable: $$LRELEASE_EXECUTABLE)
}

message (PREFIX=$$PREFIX)
message (BINDIR=$$BINDIR)
message (DATADIR=$$DATADIR)
message (PLUGINDIR=$$PLUGINDIR)
equals (DISABLE_WIDGETS,1):message ("QtWidgets are disabled!")

#Some conf to redirect intermediate stuff in separate dirs
UI_DIR=./.build/ui/
MOC_DIR=./.build/moc/
OBJECTS_DIR=./.build/obj
RCC_DIR=./.build/rcc

