#===========================================
#  qalarm source code
#  Copyright (c) 2017, q5sys
#  Available under the -clause BSD license
#  See the LICENSE file for full details
#===========================================

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = trayalarm
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

RESOURCES += \
    media.qrc
