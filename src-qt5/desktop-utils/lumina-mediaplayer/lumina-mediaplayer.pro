include("$${PWD}/../../OS-detect.pri")

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent


TARGET = lumina-mediaplayer
TEMPLATE = app


target.path = $${L_BINDIR}

#include all the special classes from the Lumina tree
include(../../core/libLumina/LUtils.pri) #includes LUtils
include(../../core/libLumina/LuminaXDG.pri)

SOURCES += main.cpp\
        mainUI.cpp

HEADERS  += mainUI.h

FORMS += mainUI.ui

TRANSLATIONS =  i18nl-mediap_af.ts \
                i18nl-mediap_ar.ts \
                i18nl-mediap_az.ts \
                i18nl-mediap_bg.ts \
                i18nl-mediap_bn.ts \
                i18nl-mediap_bs.ts \
                i18nl-mediap_ca.ts \
                i18nl-mediap_cs.ts \
                i18nl-mediap_cy.ts \
                i18nl-mediap_da.ts \
                i18nl-mediap_de.ts \
                i18nl-mediap_el.ts \
                i18nl-mediap_en_GB.ts \
                i18nl-mediap_en_ZA.ts \
                i18nl-mediap_es.ts \
                i18nl-mediap_et.ts \
                i18nl-mediap_eu.ts \
                i18nl-mediap_fa.ts \
                i18nl-mediap_fi.ts \
                i18nl-mediap_fr.ts \
                i18nl-mediap_fr_CA.ts \
                i18nl-mediap_gl.ts \
                i18nl-mediap_he.ts \
                i18nl-mediap_hi.ts \
                i18nl-mediap_hr.ts \
                i18nl-mediap_hu.ts \
                i18nl-mediap_id.ts \
                i18nl-mediap_is.ts \
                i18nl-mediap_it.ts \
                i18nl-mediap_ja.ts \
                i18nl-mediap_ka.ts \
                i18nl-mediap_ko.ts \
                i18nl-mediap_lt.ts \
                i18nl-mediap_lv.ts \
                i18nl-mediap_mk.ts \
                i18nl-mediap_mn.ts \
                i18nl-mediap_ms.ts \
                i18nl-mediap_mt.ts \
                i18nl-mediap_nb.ts \
                i18nl-mediap_nl.ts \
                i18nl-mediap_pa.ts \
                i18nl-mediap_pl.ts \
                i18nl-mediap_pt.ts \
                i18nl-mediap_pt_BR.ts \
                i18nl-mediap_ro.ts \
                i18nl-mediap_ru.ts \
                i18nl-mediap_sk.ts \
                i18nl-mediap_sl.ts \
                i18nl-mediap_sr.ts \
                i18nl-mediap_sv.ts \
                i18nl-mediap_sw.ts \
                i18nl-mediap_ta.ts \
                i18nl-mediap_tg.ts \
                i18nl-mediap_th.ts \
                i18nl-mediap_tr.ts \
                i18nl-mediap_uk.ts \
                i18nl-mediap_uz.ts \
                i18nl-mediap_vi.ts \
                i18nl-mediap_zh_CN.ts \
                i18nl-mediap_zh_HK.ts \
                i18nl-mediap_zh_TW.ts \
                i18nl-mediap_zu.ts 

dotrans.path=$${L_SHAREDIR}/lumina-desktop/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/i18n/

desktop.files=lumina-mediaplayer.desktop
desktop.path=$${L_SHAREDIR}/applications/

INSTALLS += target desktop

WITH_I18N{
  INSTALLS += dotrans
}
