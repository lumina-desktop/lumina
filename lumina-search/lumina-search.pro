include("$${PWD}/../OS-detect.pri")

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = lumina-search
target.path = $${L_BINDIR}


TEMPLATE = app

SOURCES += main.cpp \
		MainUI.cpp \
		Worker.cpp \
		ConfigUI.cpp

HEADERS  += MainUI.h \
		Worker.h \
		ConfigUI.h

FORMS    += MainUI.ui \
		ConfigUI.ui

LIBS     += -lLuminaUtils

DEPENDPATH	+= ../libLumina

TRANSLATIONS =  i18n/lumina-search_af.ts \
                i18n/lumina-search_ar.ts \
                i18n/lumina-search_az.ts \
                i18n/lumina-search_bg.ts \
                i18n/lumina-search_bn.ts \
                i18n/lumina-search_bs.ts \
                i18n/lumina-search_ca.ts \
                i18n/lumina-search_cs.ts \
                i18n/lumina-search_cy.ts \
                i18n/lumina-search_da.ts \
                i18n/lumina-search_de.ts \
                i18n/lumina-search_el.ts \
                i18n/lumina-search_en_GB.ts \
                i18n/lumina-search_en_ZA.ts \
                i18n/lumina-search_es.ts \
                i18n/lumina-search_et.ts \
                i18n/lumina-search_eu.ts \
                i18n/lumina-search_fa.ts \
                i18n/lumina-search_fi.ts \
                i18n/lumina-search_fr.ts \
                i18n/lumina-search_fr_CA.ts \
                i18n/lumina-search_gl.ts \
                i18n/lumina-search_he.ts \
                i18n/lumina-search_hi.ts \
                i18n/lumina-search_hr.ts \
                i18n/lumina-search_hu.ts \
                i18n/lumina-search_id.ts \
                i18n/lumina-search_is.ts \
                i18n/lumina-search_it.ts \
                i18n/lumina-search_ja.ts \
                i18n/lumina-search_ka.ts \
                i18n/lumina-search_ko.ts \
                i18n/lumina-search_lt.ts \
                i18n/lumina-search_lv.ts \
                i18n/lumina-search_mk.ts \
                i18n/lumina-search_mn.ts \
                i18n/lumina-search_ms.ts \
                i18n/lumina-search_mt.ts \
                i18n/lumina-search_nb.ts \
                i18n/lumina-search_nl.ts \
                i18n/lumina-search_pa.ts \
                i18n/lumina-search_pl.ts \
                i18n/lumina-search_pt.ts \
                i18n/lumina-search_pt_BR.ts \
                i18n/lumina-search_ro.ts \
                i18n/lumina-search_ru.ts \
                i18n/lumina-search_sk.ts \
                i18n/lumina-search_sl.ts \
                i18n/lumina-search_sr.ts \
                i18n/lumina-search_sv.ts \
                i18n/lumina-search_sw.ts \
                i18n/lumina-search_ta.ts \
                i18n/lumina-search_tg.ts \
                i18n/lumina-search_th.ts \
                i18n/lumina-search_tr.ts \
                i18n/lumina-search_uk.ts \
                i18n/lumina-search_uz.ts \
                i18n/lumina-search_vi.ts \
                i18n/lumina-search_zh_CN.ts \
                i18n/lumina-search_zh_HK.ts \
                i18n/lumina-search_zh_TW.ts \
                i18n/lumina-search_zu.ts

dotrans.path=$${L_SHAREDIR}/Lumina-DE/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/Lumina-DE/i18n/

desktop.files=lumina-search.desktop
desktop.path=$${L_SHAREDIR}/applications/

INSTALLS += target dotrans desktop

NO_I18N{
  INSTALLS -= dotrans
}