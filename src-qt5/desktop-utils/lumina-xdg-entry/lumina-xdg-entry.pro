#===========================================
#  Copyright (c) 2017, q5sys (JT)
#  Available under the MIT license
#  See the LICENSE file for full details    
#===========================================
include("$${PWD}/../../OS-detect.pri")

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lumina-xdg-entry
TEMPLATE = app
target.path = $${L_BINDIR}

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

desktop.files=lumina-xdg-entry.desktop
desktop.path=$${L_SHAREDIR}/applications/

manpage.path=$${L_MANDIR}/man8/
manpage.extra="$${MAN_ZIP} lumina-xdg-entry.8 > $(INSTALL_ROOT)$${L_MANDIR}/man8/lumina-xdg-entry.8.gz"

INSTALLS += target desktop manpage
