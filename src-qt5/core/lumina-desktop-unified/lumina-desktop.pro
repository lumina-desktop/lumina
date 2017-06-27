include($${PWD}/../../OS-detect.pri)

QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras multimedia concurrent svg



TARGET = lumina-desktop-unified
target.path = $${L_BINDIR}

#include all the special classes from the Lumina tree
include(../libLumina/ResizeMenu.pri)
include(../libLumina/LDesktopUtils.pri) #includes LUtils and LOS
include(../libLumina/LuminaXDG.pri)
#include(../libLumina/LuminaX11.pri)
include(../libLumina/LuminaSingleApplication.pri)
include(../libLumina/LuminaThemes.pri)
include(../libLumina/DesktopSettings.pri)
include(../libLumina/RootWindow.pri)
include(../libLumina/ExternalProcess.pri)
include(../libLumina/NativeWindow.pri)

#include  all the main individual source groups
include(src-screensaver/screensaver.pri)
include(src-events/events.pri)
include(src-desktop/desktop.pri)

TEMPLATE = app

SOURCES += main.cpp \
	LSession.cpp \
	BootSplash.cpp

HEADERS  += global-includes.h \
	global-objects.h \
	LSession.h \
	BootSplash.h

FORMS    +=	BootSplash.ui 



#Now include all the files for the various plugins
#include(panel-plugins/panel-plugins.pri)
#include(desktop-plugins/desktop-plugins.pri)


desktop.path = $${L_SESSDIR}
desktop.files = lumina-desktop.desktop


TRANSLATIONS =  i18n/lumina-desktop_af.ts \
                i18n/lumina-desktop_ar.ts \
                i18n/lumina-desktop_az.ts \
                i18n/lumina-desktop_bg.ts \
                i18n/lumina-desktop_bn.ts \
                i18n/lumina-desktop_bs.ts \
                i18n/lumina-desktop_ca.ts \
                i18n/lumina-desktop_cs.ts \
                i18n/lumina-desktop_cy.ts \
                i18n/lumina-desktop_da.ts \
                i18n/lumina-desktop_de.ts \
                i18n/lumina-desktop_el.ts \
                i18n/lumina-desktop_en_GB.ts \
                i18n/lumina-desktop_en_ZA.ts \
                i18n/lumina-desktop_es.ts \
                i18n/lumina-desktop_et.ts \
                i18n/lumina-desktop_eu.ts \
                i18n/lumina-desktop_fa.ts \
                i18n/lumina-desktop_fi.ts \
                i18n/lumina-desktop_fr.ts \
                i18n/lumina-desktop_fr_CA.ts \
                i18n/lumina-desktop_gl.ts \
                i18n/lumina-desktop_he.ts \
                i18n/lumina-desktop_hi.ts \
                i18n/lumina-desktop_hr.ts \
                i18n/lumina-desktop_hu.ts \
                i18n/lumina-desktop_id.ts \
                i18n/lumina-desktop_is.ts \
                i18n/lumina-desktop_it.ts \
                i18n/lumina-desktop_ja.ts \
                i18n/lumina-desktop_ka.ts \
                i18n/lumina-desktop_ko.ts \
                i18n/lumina-desktop_lt.ts \
                i18n/lumina-desktop_lv.ts \
                i18n/lumina-desktop_mk.ts \
                i18n/lumina-desktop_mn.ts \
                i18n/lumina-desktop_ms.ts \
                i18n/lumina-desktop_mt.ts \
                i18n/lumina-desktop_nb.ts \
                i18n/lumina-desktop_nl.ts \
                i18n/lumina-desktop_pa.ts \
                i18n/lumina-desktop_pl.ts \
                i18n/lumina-desktop_pt.ts \
                i18n/lumina-desktop_pt_BR.ts \
                i18n/lumina-desktop_ro.ts \
                i18n/lumina-desktop_ru.ts \
                i18n/lumina-desktop_sk.ts \
                i18n/lumina-desktop_sl.ts \
                i18n/lumina-desktop_sr.ts \
                i18n/lumina-desktop_sv.ts \
                i18n/lumina-desktop_sw.ts \
                i18n/lumina-desktop_ta.ts \
                i18n/lumina-desktop_tg.ts \
                i18n/lumina-desktop_th.ts \
                i18n/lumina-desktop_tr.ts \
                i18n/lumina-desktop_uk.ts \
                i18n/lumina-desktop_uz.ts \
                i18n/lumina-desktop_vi.ts \
                i18n/lumina-desktop_zh_CN.ts \
                i18n/lumina-desktop_zh_HK.ts \
                i18n/lumina-desktop_zh_TW.ts \
                i18n/lumina-desktop_zu.ts

dotrans.path=$${L_SHAREDIR}/lumina-desktop/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/i18n/

INSTALLS += target desktop

WITH_I18N{
  INSTALLS += dotrans
}
