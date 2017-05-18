#-------------------------------------------------
# Created by q5sys
# Released under MIT License 2017-03-08
#-------------------------------------------------
include($${PWD}/../../OS-detect.pri)

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lumina-notify
target.path = $${L_BINDIR}
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp

INSTALLS += target desktop
