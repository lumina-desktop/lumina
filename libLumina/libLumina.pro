include("$${PWD}/../OS-detect.pri")

QT       += core network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras multimedia concurrent svg

define
#Setup any special defines (qmake -> C++)
GIT_VERSION=$$system(git describe --always)
!isEmpty(GIT_VERSION){
  DEFINES += GIT_VERSION='"\\\"$${GIT_VERSION}\\\""'
}
DEFINES += BUILD_DATE='"\\\"$$system(date)\\\""'

TARGET=LuminaUtils

target.path = $${L_LIBDIR}

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

# Also load the OS template as available for
# LuminaOS support functions (or fall back to generic one)
exists($${PWD}/LuminaOS-$${LINUX_DISTRO}.cpp){
  SOURCES += LuminaOS-$${LINUX_DISTRO}.cpp
}else:exists($${PWD}/LuminaOS-$${OS}.cpp){
  SOURCES += LuminaOS-$${OS}.cpp
}else{
  SOURCES += LuminaOS-template.cpp
}

LIBS	+= -lc -lxcb -lxcb-ewmh -lxcb-icccm -lxcb-image -lxcb-composite -lxcb-damage -lxcb-util -lXdamage 

include.path=$${L_INCLUDEDIR}
include.files=LuminaXDG.h \
	LuminaUtils.h \
	LuminaX11.h \
	LuminaThemes.h \
	LuminaOS.h \
	LuminaSingleApplication.h

colors.path=$${L_SHAREDIR}/Lumina-DE/colors
colors.files=colors/*.qss.colors

themes.path=$${L_SHAREDIR}/Lumina-DE/themes/
themes.files=themes/*.qss.template

quickplugins.path=$${L_SHAREDIR}/Lumina-DE/quickplugins/
quickplugins.files=quickplugins/*

INSTALLS += target include colors themes quickplugins
