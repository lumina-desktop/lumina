include("$${PWD}/../OS-detect.pri")

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras


TARGET = lumina-screenshot
target.path = $${L_BINDIR}

TEMPLATE = app

SOURCES += main.cpp \
		MainUI.cpp

HEADERS  += MainUI.h

FORMS    += MainUI.ui

LIBS     += -lLuminaUtils

DEPENDPATH	+= ../libLumina

TRANSLATIONS =  i18n/lumina-screenshot_af.ts \
                i18n/lumina-screenshot_ar.ts \
                i18n/lumina-screenshot_az.ts \
                i18n/lumina-screenshot_bg.ts \
                i18n/lumina-screenshot_bn.ts \
                i18n/lumina-screenshot_bs.ts \
                i18n/lumina-screenshot_ca.ts \
                i18n/lumina-screenshot_cs.ts \
                i18n/lumina-screenshot_cy.ts \
                i18n/lumina-screenshot_da.ts \
                i18n/lumina-screenshot_de.ts \
                i18n/lumina-screenshot_el.ts \
                i18n/lumina-screenshot_en_GB.ts \
                i18n/lumina-screenshot_en_ZA.ts \
                i18n/lumina-screenshot_es.ts \
                i18n/lumina-screenshot_et.ts \
                i18n/lumina-screenshot_eu.ts \
                i18n/lumina-screenshot_fa.ts \
                i18n/lumina-screenshot_fi.ts \
                i18n/lumina-screenshot_fr.ts \
                i18n/lumina-screenshot_fr_CA.ts \
                i18n/lumina-screenshot_gl.ts \
                i18n/lumina-screenshot_he.ts \
                i18n/lumina-screenshot_hi.ts \
                i18n/lumina-screenshot_hr.ts \
                i18n/lumina-screenshot_hu.ts \
                i18n/lumina-screenshot_id.ts \
                i18n/lumina-screenshot_is.ts \
                i18n/lumina-screenshot_it.ts \
                i18n/lumina-screenshot_ja.ts \
                i18n/lumina-screenshot_ka.ts \
                i18n/lumina-screenshot_ko.ts \
                i18n/lumina-screenshot_lt.ts \
                i18n/lumina-screenshot_lv.ts \
                i18n/lumina-screenshot_mk.ts \
                i18n/lumina-screenshot_mn.ts \
                i18n/lumina-screenshot_ms.ts \
                i18n/lumina-screenshot_mt.ts \
                i18n/lumina-screenshot_nb.ts \
                i18n/lumina-screenshot_nl.ts \
                i18n/lumina-screenshot_pa.ts \
                i18n/lumina-screenshot_pl.ts \
                i18n/lumina-screenshot_pt.ts \
                i18n/lumina-screenshot_pt_BR.ts \
                i18n/lumina-screenshot_ro.ts \
                i18n/lumina-screenshot_ru.ts \
                i18n/lumina-screenshot_sk.ts \
                i18n/lumina-screenshot_sl.ts \
                i18n/lumina-screenshot_sr.ts \
                i18n/lumina-screenshot_sv.ts \
                i18n/lumina-screenshot_sw.ts \
                i18n/lumina-screenshot_ta.ts \
                i18n/lumina-screenshot_tg.ts \
                i18n/lumina-screenshot_th.ts \
                i18n/lumina-screenshot_tr.ts \
                i18n/lumina-screenshot_uk.ts \
                i18n/lumina-screenshot_uz.ts \
                i18n/lumina-screenshot_vi.ts \
                i18n/lumina-screenshot_zh_CN.ts \
                i18n/lumina-screenshot_zh_HK.ts \
                i18n/lumina-screenshot_zh_TW.ts \
                i18n/lumina-screenshot_zu.ts

dotrans.path=$${L_SHAREDIR}/Lumina-DE/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/Lumina-DE/i18n/

desktop.files=lumina-screenshot.desktop
desktop.path=$${L_SHAREDIR}/applications/

INSTALLS += target dotrans desktop

NO_I18N{
  INSTALLS -= dotrans
}