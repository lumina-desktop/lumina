include("$${PWD}/../../OS-detect.pri")

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = lumina-pingcursor
target.path = $${L_BINDIR}

TEMPLATE = app

SOURCES += main.cpp

INSTALLS += target 
