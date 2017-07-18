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

message(generating translations)
unix:system(find . -name *.ts | xargs $$LRELEASE_EXECUTABLE)

include(lthemeengine.pri)

message (PREFIX=$$PREFIX)
message (BINDIR=$$BINDIR)
message (DATADIR=$$DATADIR)
message (PLUGINDIR=$$PLUGINDIR)
equals (DISABLE_WIDGETS,1):message ("QtWidgets are disabled!")
