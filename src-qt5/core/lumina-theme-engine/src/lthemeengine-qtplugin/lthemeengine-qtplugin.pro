include(../../lthemeengine.pri)

TEMPLATE = lib
TARGET = lthemeengine
CONFIG *= plugin

greaterThan(QT_MINOR_VERSION, 7) {
  QT += gui-private theme_support-private
} else {
  QT += gui-private platformsupport-private
}

!contains(DEFINES, QT_NO_DBUS):greaterThan(QT_MINOR_VERSION, 5) {
    QT += dbus
}

SOURCES += \
    main.cpp \
    lthemeengineplatformtheme.cpp \
    ../lthemeengine/lthemeengine.cpp

OTHER_FILES += lthemeengine.json

LIBS += -lXcursor
QT += x11extras

INCLUDEPATH += ../

HEADERS += \
    lthemeengineplatformtheme.h \
    ../lthemeengine/lthemeengine.h

target.path = $$PLUGINDIR/platformthemes
INSTALLS += target
