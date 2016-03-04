include("$${PWD}/../OS-detect.pri")

QT       += core gui widgets network


TARGET = lumina-info
target.path = $${L_BINDIR}

TEMPLATE = app

SOURCES += main.cpp \
		MainUI.cpp

HEADERS  += MainUI.h

FORMS    += MainUI.ui

RESOURCES+= lumina-info.qrc

LIBS     += -lLuminaUtils

DEPENDPATH	+= ../libLumina

TRANSLATIONS =  i18n/lumina-info_af.ts \
                i18n/lumina-info_ar.ts \
                i18n/lumina-info_az.ts \
                i18n/lumina-info_bg.ts \
                i18n/lumina-info_bn.ts \
                i18n/lumina-info_bs.ts \
                i18n/lumina-info_ca.ts \
                i18n/lumina-info_cs.ts \
                i18n/lumina-info_cy.ts \
                i18n/lumina-info_da.ts \
                i18n/lumina-info_de.ts \
                i18n/lumina-info_el.ts \
                i18n/lumina-info_en_GB.ts \
                i18n/lumina-info_en_ZA.ts \
                i18n/lumina-info_es.ts \
                i18n/lumina-info_et.ts \
                i18n/lumina-info_eu.ts \
                i18n/lumina-info_fa.ts \
                i18n/lumina-info_fi.ts \
                i18n/lumina-info_fr.ts \
                i18n/lumina-info_fr_CA.ts \
                i18n/lumina-info_gl.ts \
                i18n/lumina-info_he.ts \
                i18n/lumina-info_hi.ts \
                i18n/lumina-info_hr.ts \
                i18n/lumina-info_hu.ts \
                i18n/lumina-info_id.ts \
                i18n/lumina-info_is.ts \
                i18n/lumina-info_it.ts \
                i18n/lumina-info_ja.ts \
                i18n/lumina-info_ka.ts \
                i18n/lumina-info_ko.ts \
                i18n/lumina-info_lt.ts \
                i18n/lumina-info_lv.ts \
                i18n/lumina-info_mk.ts \
                i18n/lumina-info_mn.ts \
                i18n/lumina-info_ms.ts \
                i18n/lumina-info_mt.ts \
                i18n/lumina-info_nb.ts \
                i18n/lumina-info_nl.ts \
                i18n/lumina-info_pa.ts \
                i18n/lumina-info_pl.ts \
                i18n/lumina-info_pt.ts \
                i18n/lumina-info_pt_BR.ts \
                i18n/lumina-info_ro.ts \
                i18n/lumina-info_ru.ts \
                i18n/lumina-info_sk.ts \
                i18n/lumina-info_sl.ts \
                i18n/lumina-info_sr.ts \
                i18n/lumina-info_sv.ts \
                i18n/lumina-info_sw.ts \
                i18n/lumina-info_ta.ts \
                i18n/lumina-info_tg.ts \
                i18n/lumina-info_th.ts \
                i18n/lumina-info_tr.ts \
                i18n/lumina-info_uk.ts \
                i18n/lumina-info_uz.ts \
                i18n/lumina-info_vi.ts \
                i18n/lumina-info_zh_CN.ts \
                i18n/lumina-info_zh_HK.ts \
                i18n/lumina-info_zh_TW.ts \
                i18n/lumina-info_zu.ts

dotrans.path=$${L_SHAREDIR}/Lumina-DE/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/Lumina-DE/i18n/

desktop.files=lumina-info.desktop lumina-support.desktop
desktop.path=$${L_SHAREDIR}/applications/

INSTALLS += target dotrans desktop

NO_I18N{
  INSTALLS -= dotrans
}