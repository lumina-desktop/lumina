#-------------------------------------------------
#
# Project created by QtCreator 2015-02-24T18:52:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = desktop-editor
TEMPLATE = app

isEmpty(PREFIX) {
 PREFIX = /usr/local
}

isEmpty(LIBPREFIX) {
 LIBPREFIX = $$PREFIX/lib
}


SOURCES += main.cpp\
        dialog.cpp

HEADERS  += dialog.h

FORMS    += dialog.ui

INCLUDEPATH +=  $$PREFIX/include

LIBS     += -L$$LIBPREFIX -lLuminaUtils
