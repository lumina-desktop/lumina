include("$${PWD}/../OS-detect.pri")

QT       += core network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras multimedia concurrent svg


TARGET=LuminaUtils

system(./make-global-h.sh $$PREFIX)

target.path = $$DESTDIR$$LIBPREFIX

DESTDIR= $$_PRO_FILE_PWD_/

TEMPLATE	= lib
LANGUAGE	= C++
VERSION		= 1

HEADERS	+= LuminaXDG.h \
	LuminaUtils.h \
	LuminaX11.h \
	LuminaThemes.h \
	LuminaOS.h \
	LuminaSingleApplication.h

SOURCES	+= LuminaXDG.cpp \
	LuminaUtils.cpp \
	LuminaX11.cpp \
	LuminaThemes.cpp \
	LuminaSingleApplication.cpp
#	LuminaOS-FreeBSD.cpp \
#	LuminaOS-DragonFly.cpp \
#	LuminaOS-NetBSD.cpp \
#	LuminaOS-OpenBSD.cpp \
#     LuminaOS-kFreeBSD.cpp
#       new OS support can be added here

# check linux distribution and use specific
# LuminaOS support functions (or fall back to generic one)
exists($${PWD}/LuminaOS-$${LINUX_DISTRO}.cpp){
  SOURCES += LuminaOS-$${LINUX_DISTRO}.cpp
}else:exists($${PWD}/LuminaOS-$${OS}.cpp){
  SOURCES += LuminaOS-$${OS}.cpp
}else{
  SOURCES += LuminaOS-template.cpp
}

INCLUDEPATH += $$PREFIX/include

LIBS	+= -lc -lxcb -lxcb-ewmh -lxcb-icccm -lxcb-image -lxcb-composite -lxcb-damage -lxcb-util -lXdamage 

include.path=$$PREFIX/include/
include.files=LuminaXDG.h \
	LuminaUtils.h \
	LuminaX11.h \
	LuminaThemes.h \
	LuminaOS.h \
	LuminaSingleApplication.h

colors.path=$$PREFIX/share/Lumina-DE/colors/
colors.files=colors/*.qss.colors

themes.path=$$PREFIX/share/Lumina-DE/themes/
themes.files=themes/*.qss.template

quickplugins.path=$$PREFIX/share/Lumina-DE/quickplugins/
quickplugins.files=quickplugins/*

INSTALLS += target include colors themes quickplugins

QMAKE_LIBDIR = $$LIBPREFIX/qt5 $$LIBPREFIX
