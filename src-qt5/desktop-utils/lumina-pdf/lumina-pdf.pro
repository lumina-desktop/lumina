include("$${PWD}/../../OS-detect.pri")

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent


TARGET = lumina-pdf
TEMPLATE = app

QT += printsupport

target.path = $${L_BINDIR}

#include all the special classes from the Lumina tree
include(../../core/libLumina/LUtils.pri) #includes LUtils
include(../../core/libLumina/LuminaXDG.pri)

SOURCES += main.cpp\
        mainUI.cpp

HEADERS  += mainUI.h

FORMS += mainUI.ui

LIBS += -lpoppler-qt5
INCLUDEPATH+= $${L_INCLUDEDIR}/poppler/qt5

TRANSLATIONS =  i18n/l-pdf_af.ts \
                i18n/l-pdf_ar.ts \
                i18n/l-pdf_az.ts \
                i18n/l-pdf_bg.ts \
                i18n/l-pdf_bn.ts \
                i18n/l-pdf_bs.ts \
                i18n/l-pdf_ca.ts \
                i18n/l-pdf_cs.ts \
                i18n/l-pdf_cy.ts \
                i18n/l-pdf_da.ts \
                i18n/l-pdf_de.ts \
                i18n/l-pdf_el.ts \
                i18n/l-pdf_en_GB.ts \
                i18n/l-pdf_en_ZA.ts \
                i18n/l-pdf_es.ts \
                i18n/l-pdf_et.ts \
                i18n/l-pdf_eu.ts \
                i18n/l-pdf_fa.ts \
                i18n/l-pdf_fi.ts \
                i18n/l-pdf_fr.ts \
                i18n/l-pdf_fr_CA.ts \
                i18n/l-pdf_gl.ts \
                i18n/l-pdf_he.ts \
                i18n/l-pdf_hi.ts \
                i18n/l-pdf_hr.ts \
                i18n/l-pdf_hu.ts \
                i18n/l-pdf_id.ts \
                i18n/l-pdf_is.ts \
                i18n/l-pdf_it.ts \
                i18n/l-pdf_ja.ts \
                i18n/l-pdf_ka.ts \
                i18n/l-pdf_ko.ts \
                i18n/l-pdf_lt.ts \
                i18n/l-pdf_lv.ts \
                i18n/l-pdf_mk.ts \
                i18n/l-pdf_mn.ts \
                i18n/l-pdf_ms.ts \
                i18n/l-pdf_mt.ts \
                i18n/l-pdf_nb.ts \
                i18n/l-pdf_nl.ts \
                i18n/l-pdf_pa.ts \
                i18n/l-pdf_pl.ts \
                i18n/l-pdf_pt.ts \
                i18n/l-pdf_pt_BR.ts \
                i18n/l-pdf_ro.ts \
                i18n/l-pdf_ru.ts \
                i18n/l-pdf_sk.ts \
                i18n/l-pdf_sl.ts \
                i18n/l-pdf_sr.ts \
                i18n/l-pdf_sv.ts \
                i18n/l-pdf_sw.ts \
                i18n/l-pdf_ta.ts \
                i18n/l-pdf_tg.ts \
                i18n/l-pdf_th.ts \
                i18n/l-pdf_tr.ts \
                i18n/l-pdf_uk.ts \
                i18n/l-pdf_uz.ts \
                i18n/l-pdf_vi.ts \
                i18n/l-pdf_zh_CN.ts \
                i18n/l-pdf_zh_HK.ts \
                i18n/l-pdf_zh_TW.ts \
                i18n/l-pdf_zu.ts 

dotrans.path=$${L_SHAREDIR}/lumina-desktop/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/i18n/

desktop.files=lumina-pdf.desktop
desktop.path=$${L_SHAREDIR}/applications/

INSTALLS += target desktop

WITH_I18N{
  INSTALLS += dotrans
}
