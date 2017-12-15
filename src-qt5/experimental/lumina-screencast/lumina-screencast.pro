include("$${PWD}/../../OS-detect.pri")

QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras

TARGET = lumina-screencast
target.path = $${L_BINDIR}

TEMPLATE = app

#include all the special classes from the Lumina tree
include(../../core/libLumina/LUtils.pri) #includes LUtils
include(../../core/libLumina/LuminaXDG.pri)
include(../../core/libLumina/LuminaSingleApplication.pri)
include(../../core/libLumina/LuminaThemes.pri)
include(../../core/libLumina/LuminaX11.pri)

SOURCES += main.cpp \
		MainUI.cpp \

HEADERS  += MainUI.h \

FORMS    += MainUI.ui

TRANSLATIONS =  i18n/l-screencast_af.ts \
                i18n/l-screencast_ar.ts \
                i18n/l-screencast_az.ts \
                i18n/l-screencast_bg.ts \
                i18n/l-screencast_bn.ts \
                i18n/l-screencast_bs.ts \
                i18n/l-screencast_ca.ts \
                i18n/l-screencast_cs.ts \
                i18n/l-screencast_cy.ts \
                i18n/l-screencast_da.ts \
                i18n/l-screencast_de.ts \
                i18n/l-screencast_el.ts \
                i18n/l-screencast_en_GB.ts \
                i18n/l-screencast_en_ZA.ts \
                i18n/l-screencast_en_AU.ts \
                i18n/l-screencast_es.ts \
                i18n/l-screencast_et.ts \
                i18n/l-screencast_eu.ts \
                i18n/l-screencast_fa.ts \
                i18n/l-screencast_fi.ts \
                i18n/l-screencast_fr.ts \
                i18n/l-screencast_fr_CA.ts \
                i18n/l-screencast_gl.ts \
                i18n/l-screencast_he.ts \
                i18n/l-screencast_hi.ts \
                i18n/l-screencast_hr.ts \
                i18n/l-screencast_hu.ts \
                i18n/l-screencast_id.ts \
                i18n/l-screencast_is.ts \
                i18n/l-screencast_it.ts \
                i18n/l-screencast_ja.ts \
                i18n/l-screencast_ka.ts \
                i18n/l-screencast_ko.ts \
                i18n/l-screencast_lt.ts \
                i18n/l-screencast_lv.ts \
                i18n/l-screencast_mk.ts \
                i18n/l-screencast_mn.ts \
                i18n/l-screencast_ms.ts \
                i18n/l-screencast_mt.ts \
                i18n/l-screencast_nb.ts \
                i18n/l-screencast_nl.ts \
                i18n/l-screencast_pa.ts \
                i18n/l-screencast_pl.ts \
                i18n/l-screencast_pt.ts \
                i18n/l-screencast_pt_BR.ts \
                i18n/l-screencast_ro.ts \
                i18n/l-screencast_ru.ts \
                i18n/l-screencast_sk.ts \
                i18n/l-screencast_sl.ts \
                i18n/l-screencast_sr.ts \
                i18n/l-screencast_sv.ts \
                i18n/l-screencast_sw.ts \
                i18n/l-screencast_ta.ts \
                i18n/l-screencast_tg.ts \
                i18n/l-screencast_th.ts \
                i18n/l-screencast_tr.ts \
                i18n/l-screencast_uk.ts \
                i18n/l-screencast_uz.ts \
                i18n/l-screencast_vi.ts \
                i18n/l-screencast_zh_CN.ts \
                i18n/l-screencast_zh_HK.ts \
                i18n/l-screencast_zh_TW.ts \
                i18n/l-screencast_zu.ts

dotrans.path=$${L_SHAREDIR}/lumina-desktop/i18n/
dotrans.extra=cd $$PWD/i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/i18n/

desktop.files=lumina-screencast.desktop
desktop.path=$${L_SHAREDIR}/applications/

INSTALLS += target desktop

WITH_I18N{
  INSTALLS += dotrans
}
