include("$${PWD}/../../../OS-detect.pri")

QT       += core gui widgets x11extras

TARGET = lcc-panels
target.path = $${L_BINDIR}

TEMPLATE = app

#include all the special classes from the Lumina tree
include(../../../core/libLumina/LDesktopUtils.pri) #includes LUtils
include(../../../core/libLumina/LuminaXDG.pri)
include(../../../core/libLumina/LuminaSingleApplication.pri)
include(../../../core/libLumina/LuminaThemes.pri)

SOURCES += main.cpp \
           MainWindow.cpp \
           PanelWidget.cpp \
           LPlugins.cpp \
           GetPluginDialog.cpp \
           AppDialog.cpp \
           ScriptDialog.cpp

HEADERS  += MainWindow.h \
            PanelWidget.h \
            LPlugins.h \
            GetPluginDialog.h \
            AppDialog.h \
            ScriptDialog.h

FORMS    += MainWindow.ui \
            PanelWidget.ui \
            GetPluginDialog.ui \
            AppDialog.ui \
            ScriptDialog.ui

# Translations will be added in the future
# TRANSLATIONS = i18n/lcc-panels_en.ts

# dotrans.path=$${L_SHAREDIR}/lumina-desktop/i18n/
# dotrans.extra=cd $$PWD/i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/i18n/

desktop.files=lcc-panels.desktop
desktop.path=$${L_SHAREDIR}/applications/

manpage.path=$${L_MANDIR}/man1/
manpage.extra="$${MAN_ZIP} $$PWD/lcc-panels.1 > $(INSTALL_ROOT)$${L_MANDIR}/man1/lcc-panels.1.gz"

INSTALLS += target desktop

WITH_I18N{
  INSTALLS += dotrans
}

WITH_MANPAGES{
  INSTALLS += manpage
}
