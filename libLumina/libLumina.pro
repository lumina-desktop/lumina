
QT       += core

TARGET=LuminaUtils

isEmpty(PREFIX) {
 PREFIX = /usr/local
}

isEmtpy(LIBPREFIX) {
 LIBPREFIX = $$PREFIX/lib
}
target.path = $$LIBPREFIX

DESTDIR= $$_PRO_FILE_PWD_/

TEMPLATE	= lib
LANGUAGE	= C++
VERSION		= 1

HEADERS	+= LuminaXDG.h \
	LuminaUtils.h \
	LuminaX11.h \
	LuminaOS.h

SOURCES	+= LuminaXDG.cpp \
	LuminaUtils.cpp \
	LuminaX11.cpp \
	LuminaOS-FreeBSD.cpp \
	LuminaOS-DragonFly.cpp \
	LuminaOS-OpenBSD.cpp \
	LuminaOS-Linux.cpp \
        LuminaOS-kFreeBSD.cpp
#       new OS support can be added here

INCLUDEPATH += $$PREFIX/include

LIBS	+= -lX11 -lXrender -lXcomposite

include.path=$$PREFIX/include/
include.files=LuminaXDG.h \
	LuminaUtils.h \
	LuminaX11.h \
	LuminaOS.h

INSTALLS += target include

QMAKE_LIBDIR = $$LIBPREFIX/qt4 $$LIBPREFIX
