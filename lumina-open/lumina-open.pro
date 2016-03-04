include("$${PWD}/../OS-detect.pri")

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras


TARGET = lumina-open
target.path = $${L_BINDIR}

TEMPLATE = app

SOURCES += main.cpp \
	   LFileDialog.cpp

HEADERS  += LFileDialog.h

FORMS    += LFileDialog.ui

RESOURCES+= lumina-open.qrc

LIBS     += -lLuminaUtils

DEPENDPATH	+= ../libLumina


TRANSLATIONS =  i18n/lumina-open_af.ts \
                i18n/lumina-open_ar.ts \
                i18n/lumina-open_az.ts \
                i18n/lumina-open_bg.ts \
                i18n/lumina-open_bn.ts \
                i18n/lumina-open_bs.ts \
                i18n/lumina-open_ca.ts \
                i18n/lumina-open_cs.ts \
                i18n/lumina-open_cy.ts \
                i18n/lumina-open_da.ts \
                i18n/lumina-open_de.ts \
                i18n/lumina-open_el.ts \
                i18n/lumina-open_en_GB.ts \
                i18n/lumina-open_en_ZA.ts \
                i18n/lumina-open_es.ts \
                i18n/lumina-open_et.ts \
                i18n/lumina-open_eu.ts \
                i18n/lumina-open_fa.ts \
                i18n/lumina-open_fi.ts \
                i18n/lumina-open_fr.ts \
                i18n/lumina-open_fr_CA.ts \
                i18n/lumina-open_gl.ts \
                i18n/lumina-open_he.ts \
                i18n/lumina-open_hi.ts \
                i18n/lumina-open_hr.ts \
                i18n/lumina-open_hu.ts \
                i18n/lumina-open_id.ts \
                i18n/lumina-open_is.ts \
                i18n/lumina-open_it.ts \
                i18n/lumina-open_ja.ts \
                i18n/lumina-open_ka.ts \
                i18n/lumina-open_ko.ts \
                i18n/lumina-open_lt.ts \
                i18n/lumina-open_lv.ts \
                i18n/lumina-open_mk.ts \
                i18n/lumina-open_mn.ts \
                i18n/lumina-open_ms.ts \
                i18n/lumina-open_mt.ts \
                i18n/lumina-open_nb.ts \
                i18n/lumina-open_nl.ts \
                i18n/lumina-open_pa.ts \
                i18n/lumina-open_pl.ts \
                i18n/lumina-open_pt.ts \
                i18n/lumina-open_pt_BR.ts \
                i18n/lumina-open_ro.ts \
                i18n/lumina-open_ru.ts \
                i18n/lumina-open_sk.ts \
                i18n/lumina-open_sl.ts \
                i18n/lumina-open_sr.ts \
                i18n/lumina-open_sv.ts \
                i18n/lumina-open_sw.ts \
                i18n/lumina-open_ta.ts \
                i18n/lumina-open_tg.ts \
                i18n/lumina-open_th.ts \
                i18n/lumina-open_tr.ts \
                i18n/lumina-open_uk.ts \
                i18n/lumina-open_uz.ts \
                i18n/lumina-open_vi.ts \
                i18n/lumina-open_zh_CN.ts \
                i18n/lumina-open_zh_HK.ts \
                i18n/lumina-open_zh_TW.ts \
                i18n/lumina-open_zu.ts

dotrans.path=$${L_SHAREDIR}/Lumina-DE/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/Lumina-DE/i18n/

INSTALLS += target dotrans

NO_I18N{
  INSTALLS -= dotrans
}
