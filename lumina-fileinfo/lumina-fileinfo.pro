#-------------------------------------------------
#
# Project created by QtCreator 2015-02-24T18:52:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lumina-fileinfo
TEMPLATE = app

isEmpty(PREFIX) {
 PREFIX = /usr/local
}
target.path = $$PREFIX/bin

isEmpty(LIBPREFIX) {
 LIBPREFIX = $$PREFIX/lib
}


SOURCES += main.cpp\
        dialog.cpp

HEADERS  += dialog.h

FORMS    += dialog.ui

RESOURCES+= lumina-fileinfo.qrc

INCLUDEPATH +=  $$PREFIX/include

LIBS     += -L$$LIBPREFIX -lLuminaUtils

desktop.files=lumina-search.desktop
desktop.path=$$PREFIX/share/applications/

INSTALLS += target desktop
