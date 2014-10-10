
QT       += core

TARGET=LuminaUtils

isEmpty(PREFIX) {
 PREFIX = /usr/local
}

system(./make-global-h.sh $$PREFIX)

isEmpty(LIBPREFIX) {
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
	LuminaThemes.h \
	LuminaOS.h

SOURCES	+= LuminaXDG.cpp \
	LuminaUtils.cpp \
	LuminaX11.cpp \
	LuminaThemes.cpp \
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
	LuminaThemes.h \
	LuminaOS.h

colors.path=$$PREFIX/share/Lumina-DE/colors/
colors.files=colors/SampleColors.qss.colors

themes.path=$$PREFIX/share/Lumina-DE/themes/
themes.files=themes/Lumina-default.qss.template

INSTALLS += target include colors themes

QMAKE_LIBDIR = $$LIBPREFIX/qt4 $$LIBPREFIX
