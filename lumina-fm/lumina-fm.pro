include("$${PWD}/../OS-detect.pri")

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia multimediawidgets network concurrent svg


TARGET = lumina-fm
target.path = $${L_BINDIR}

TEMPLATE = app

SOURCES += main.cpp \
		MainUI.cpp \
		FODialog.cpp \
		BMMDialog.cpp \
		widgets/MultimediaWidget.cpp \
		widgets/SlideshowWidget.cpp \
		widgets/DirWidget.cpp

HEADERS  += MainUI.h \
		FODialog.h \
		BMMDialog.h \
		ScrollDialog.h \
		DirData.h \
		widgets/DDListWidgets.h \
		widgets/MultimediaWidget.h \
		widgets/SlideshowWidget.h \
		widgets/DirWidget.h

FORMS    += MainUI.ui \
		FODialog.ui \
		BMMDialog.ui \
		widgets/MultimediaWidget.ui \
		widgets/SlideshowWidget.ui \
		widgets/DirWidget.ui

# RESOURCES+= lumina-fm.qrc

LIBS     += -lLuminaUtils

DEPENDPATH	+= ../libLumina

TRANSLATIONS =  i18n/lumina-fm_af.ts \
                i18n/lumina-fm_ar.ts \
                i18n/lumina-fm_az.ts \
                i18n/lumina-fm_bg.ts \
                i18n/lumina-fm_bn.ts \
                i18n/lumina-fm_bs.ts \
                i18n/lumina-fm_ca.ts \
                i18n/lumina-fm_cs.ts \
                i18n/lumina-fm_cy.ts \
                i18n/lumina-fm_da.ts \
                i18n/lumina-fm_de.ts \
                i18n/lumina-fm_el.ts \
                i18n/lumina-fm_en_GB.ts \
                i18n/lumina-fm_en_ZA.ts \
                i18n/lumina-fm_es.ts \
                i18n/lumina-fm_et.ts \
                i18n/lumina-fm_eu.ts \
                i18n/lumina-fm_fa.ts \
                i18n/lumina-fm_fi.ts \
                i18n/lumina-fm_fr.ts \
                i18n/lumina-fm_fr_CA.ts \
                i18n/lumina-fm_gl.ts \
                i18n/lumina-fm_he.ts \
                i18n/lumina-fm_hi.ts \
                i18n/lumina-fm_hr.ts \
                i18n/lumina-fm_hu.ts \
                i18n/lumina-fm_id.ts \
                i18n/lumina-fm_is.ts \
                i18n/lumina-fm_it.ts \
                i18n/lumina-fm_ja.ts \
                i18n/lumina-fm_ka.ts \
                i18n/lumina-fm_ko.ts \
                i18n/lumina-fm_lt.ts \
                i18n/lumina-fm_lv.ts \
                i18n/lumina-fm_mk.ts \
                i18n/lumina-fm_mn.ts \
                i18n/lumina-fm_ms.ts \
                i18n/lumina-fm_mt.ts \
                i18n/lumina-fm_nb.ts \
                i18n/lumina-fm_nl.ts \
                i18n/lumina-fm_pa.ts \
                i18n/lumina-fm_pl.ts \
                i18n/lumina-fm_pt.ts \
                i18n/lumina-fm_pt_BR.ts \
                i18n/lumina-fm_ro.ts \
                i18n/lumina-fm_ru.ts \
                i18n/lumina-fm_sk.ts \
                i18n/lumina-fm_sl.ts \
                i18n/lumina-fm_sr.ts \
                i18n/lumina-fm_sv.ts \
                i18n/lumina-fm_sw.ts \
                i18n/lumina-fm_ta.ts \
                i18n/lumina-fm_tg.ts \
                i18n/lumina-fm_th.ts \
                i18n/lumina-fm_tr.ts \
                i18n/lumina-fm_uk.ts \
                i18n/lumina-fm_uz.ts \
                i18n/lumina-fm_vi.ts \
                i18n/lumina-fm_zh_CN.ts \
                i18n/lumina-fm_zh_HK.ts \
                i18n/lumina-fm_zh_TW.ts \
                i18n/lumina-fm_zu.ts

dotrans.path=$${L_SHAREDIR}/Lumina-DE/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/Lumina-DE/i18n/

desktop.files=lumina-fm.desktop
desktop.path=$${L_SHAREDIR}/applications/

INSTALLS += target dotrans desktop

NO_I18N{
  INSTALLS -= dotrans
}
