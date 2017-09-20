include(../../lthemeengine.pri)

TEMPLATE = app

QT *= widgets

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    tabpage.cpp \
    appearancepage.cpp \
    fontspage.cpp \
    lthemeengine.cpp \
    paletteeditdialog.cpp \
    iconthemepage.cpp \
    interfacepage.cpp \
    fontconfigdialog.cpp \
    qsspage.cpp \
    qsseditordialog.cpp \
    cursorthemepage.cpp

FORMS += \
    mainwindow.ui \
    appearancepage.ui \
    fontspage.ui \
    paletteeditdialog.ui \
    iconthemepage.ui \
    interfacepage.ui \
    fontconfigdialog.ui \
    previewform.ui \
    qsspage.ui \
    qsseditordialog.ui \
    cursorthemepage.ui

HEADERS += \
    mainwindow.h \
    tabpage.h \
    appearancepage.h \
    fontspage.h \
    lthemeengine.h \
    paletteeditdialog.h \
    iconthemepage.h \
    interfacepage.h \
    fontconfigdialog.h \
    qsspage.h \
    qsseditordialog.h \
    cursorthemepage.h

   DEFINES += USE_WIDGETS

TARGET = lthemeengine
target.path = $${L_BINDIR}

desktop.files = lthemeengine.desktop
desktop.path = $${L_SHAREDIR}/applications

INSTALLS += target desktop
