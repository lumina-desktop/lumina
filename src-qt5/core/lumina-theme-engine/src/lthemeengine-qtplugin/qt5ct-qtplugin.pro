include(../../qt5ct.pri)

TEMPLATE = lib
TARGET = qt5ct
CONFIG += plugin

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
    qt5ctplatformtheme.cpp \
    ../qt5ct/qt5ct.cpp

!equals (DISABLE_WIDGETS,1) {
   QT += widgets
}

OTHER_FILES += qt5ct.json

INCLUDEPATH += ../

HEADERS += \
    qt5ctplatformtheme.h \
    ../qt5ct/qt5ct.h

target.path = $$PLUGINDIR/platformthemes
INSTALLS += target
