include(../../lthemeengine.pri)

TEMPLATE = lib
TARGET = lthemeengine-style
QT += widgets

# Input

CONFIG += plugin

target.path = $$PLUGINDIR/styles
INSTALLS += target

INCLUDEPATH += ../

HEADERS += \
    lthemeengineproxystyle.h \
    ../lthemeengine/qt5ct.h

SOURCES += \
    plugin.cpp \
    lthemeengineproxystyle.cpp \
    ../lthemeengine/qt5ct.cpp

OTHER_FILES += \
    lthemeengine.json
