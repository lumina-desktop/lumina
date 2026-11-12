include(../../lthemeengine.pri)
TEMPLATE = app
QT *= widgets

SOURCES += \
    main.cpp

TARGET = lthemeengine-sstest
target.path = $${L_BINDIR}

INSTALLS += target
