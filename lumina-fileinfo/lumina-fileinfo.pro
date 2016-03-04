include("$${PWD}/../OS-detect.pri")

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent


TARGET = lumina-fileinfo
TEMPLATE = app

target.path = $${L_BINDIR}

SOURCES += main.cpp\
        MainUI.cpp

HEADERS  += MainUI.h

FORMS    += MainUI.ui

#RESOURCES+= lumina-fileinfo.qrc

LIBS += -lLuminaUtils

DEPENDPATH	+= ../libLumina


TRANSLATIONS =  i18n/lumina-fileinfo_af.ts \
                i18n/lumina-fileinfo_ar.ts \
                i18n/lumina-fileinfo_az.ts \
                i18n/lumina-fileinfo_bg.ts \
                i18n/lumina-fileinfo_bn.ts \
                i18n/lumina-fileinfo_bs.ts \
                i18n/lumina-fileinfo_ca.ts \
                i18n/lumina-fileinfo_cs.ts \
                i18n/lumina-fileinfo_cy.ts \
                i18n/lumina-fileinfo_da.ts \
                i18n/lumina-fileinfo_de.ts \
                i18n/lumina-fileinfo_el.ts \
                i18n/lumina-fileinfo_en_GB.ts \
                i18n/lumina-fileinfo_en_ZA.ts \
                i18n/lumina-fileinfo_es.ts \
                i18n/lumina-fileinfo_et.ts \
                i18n/lumina-fileinfo_eu.ts \
                i18n/lumina-fileinfo_fa.ts \
                i18n/lumina-fileinfo_fi.ts \
                i18n/lumina-fileinfo_fr.ts \
                i18n/lumina-fileinfo_fr_CA.ts \
                i18n/lumina-fileinfo_gl.ts \
                i18n/lumina-fileinfo_he.ts \
                i18n/lumina-fileinfo_hi.ts \
                i18n/lumina-fileinfo_hr.ts \
                i18n/lumina-fileinfo_hu.ts \
                i18n/lumina-fileinfo_id.ts \
                i18n/lumina-fileinfo_is.ts \
                i18n/lumina-fileinfo_it.ts \
                i18n/lumina-fileinfo_ja.ts \
                i18n/lumina-fileinfo_ka.ts \
                i18n/lumina-fileinfo_ko.ts \
                i18n/lumina-fileinfo_lt.ts \
                i18n/lumina-fileinfo_lv.ts \
                i18n/lumina-fileinfo_mk.ts \
                i18n/lumina-fileinfo_mn.ts \
                i18n/lumina-fileinfo_ms.ts \
                i18n/lumina-fileinfo_mt.ts \
                i18n/lumina-fileinfo_nb.ts \
                i18n/lumina-fileinfo_nl.ts \
                i18n/lumina-fileinfo_pa.ts \
                i18n/lumina-fileinfo_pl.ts \
                i18n/lumina-fileinfo_pt.ts \
                i18n/lumina-fileinfo_pt_BR.ts \
                i18n/lumina-fileinfo_ro.ts \
                i18n/lumina-fileinfo_ru.ts \
                i18n/lumina-fileinfo_sk.ts \
                i18n/lumina-fileinfo_sl.ts \
                i18n/lumina-fileinfo_sr.ts \
                i18n/lumina-fileinfo_sv.ts \
                i18n/lumina-fileinfo_sw.ts \
                i18n/lumina-fileinfo_ta.ts \
                i18n/lumina-fileinfo_tg.ts \
                i18n/lumina-fileinfo_th.ts \
                i18n/lumina-fileinfo_tr.ts \
                i18n/lumina-fileinfo_uk.ts \
                i18n/lumina-fileinfo_uz.ts \
                i18n/lumina-fileinfo_vi.ts \
                i18n/lumina-fileinfo_zh_CN.ts \
                i18n/lumina-fileinfo_zh_HK.ts \
                i18n/lumina-fileinfo_zh_TW.ts \
                i18n/lumina-fileinfo_zu.ts 

dotrans.path=$${L_SHAREDIR}/Lumina-DE/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/Lumina-DE/i18n/

INSTALLS += target dotrans

NO_I18N{
  INSTALLS -= dotrans
}