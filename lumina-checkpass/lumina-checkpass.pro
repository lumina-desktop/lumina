TEMPLATE = app
#Don't need any Qt - just a simple C program
QT = 
CONFIG += console

TARGET = lumina-checkpass
isEmpty(PREFIX) {
 PREFIX = /usr/local
}
target.path = $$DESTDIR$$PREFIX/sbin

isEmpty(LIBPREFIX) {
 LIBPREFIX = $$PREFIX/lib
}

LIBS     += -L$$LIBPREFIX -lpam

SOURCES += main.c

INCLUDEPATH += $$PREFIX/include

perms.path = $$DESTDIR$$PREFIX/sbin
perms.extra = "chmod 4555 $$DESTDIR$$PREFIX/sbin/lumina-checkpass"

INSTALLS += target perms

