include("$${PWD}/../../OS-detect.pri")

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent multimedia


TARGET = lumina-fileinfo
TEMPLATE = app

target.path = $${L_BINDIR}

#include all the special classes from the Lumina tree
include(../../core/libLumina/LUtils.pri) #includes LUtils
include(../../core/libLumina/LuminaXDG.pri)
#include(../../core/libLumina/LuminaSingleApplication.pri)
include(../../core/libLumina/LVideoLabel.pri)
include(../../core/libLumina/LuminaThemes.pri)

SOURCES += main.cpp\
        MainUI.cpp

HEADERS  += MainUI.h

FORMS    += MainUI.ui


TRANSLATIONS =  i18n/l-fileinfo_af.ts \
                i18n/l-fileinfo_ar.ts \
                i18n/l-fileinfo_az.ts \
                i18n/l-fileinfo_bg.ts \
                i18n/l-fileinfo_bn.ts \
                i18n/l-fileinfo_bs.ts \
                i18n/l-fileinfo_ca.ts \
                i18n/l-fileinfo_cs.ts \
                i18n/l-fileinfo_cy.ts \
                i18n/l-fileinfo_da.ts \
                i18n/l-fileinfo_de.ts \
                i18n/l-fileinfo_el.ts \
                i18n/l-fileinfo_en_GB.ts \
                i18n/l-fileinfo_en_ZA.ts \
                i18n/l-fileinfo_en_AU.ts \
                i18n/l-fileinfo_es.ts \
                i18n/l-fileinfo_et.ts \
                i18n/l-fileinfo_eu.ts \
                i18n/l-fileinfo_fa.ts \
                i18n/l-fileinfo_fi.ts \
                i18n/l-fileinfo_fr.ts \
                i18n/l-fileinfo_fr_CA.ts \
                i18n/l-fileinfo_gl.ts \
                i18n/l-fileinfo_he.ts \
                i18n/l-fileinfo_hi.ts \
                i18n/l-fileinfo_hr.ts \
                i18n/l-fileinfo_hu.ts \
                i18n/l-fileinfo_id.ts \
                i18n/l-fileinfo_is.ts \
                i18n/l-fileinfo_it.ts \
                i18n/l-fileinfo_ja.ts \
                i18n/l-fileinfo_ka.ts \
                i18n/l-fileinfo_ko.ts \
                i18n/l-fileinfo_lt.ts \
                i18n/l-fileinfo_lv.ts \
                i18n/l-fileinfo_mk.ts \
                i18n/l-fileinfo_mn.ts \
                i18n/l-fileinfo_ms.ts \
                i18n/l-fileinfo_mt.ts \
                i18n/l-fileinfo_nb.ts \
                i18n/l-fileinfo_nl.ts \
                i18n/l-fileinfo_pa.ts \
                i18n/l-fileinfo_pl.ts \
                i18n/l-fileinfo_pt.ts \
                i18n/l-fileinfo_pt_BR.ts \
                i18n/l-fileinfo_ro.ts \
                i18n/l-fileinfo_ru.ts \
                i18n/l-fileinfo_sk.ts \
                i18n/l-fileinfo_sl.ts \
                i18n/l-fileinfo_sr.ts \
                i18n/l-fileinfo_sv.ts \
                i18n/l-fileinfo_sw.ts \
                i18n/l-fileinfo_ta.ts \
                i18n/l-fileinfo_tg.ts \
                i18n/l-fileinfo_th.ts \
                i18n/l-fileinfo_tr.ts \
                i18n/l-fileinfo_uk.ts \
                i18n/l-fileinfo_uz.ts \
                i18n/l-fileinfo_vi.ts \
                i18n/l-fileinfo_zh_CN.ts \
                i18n/l-fileinfo_zh_HK.ts \
                i18n/l-fileinfo_zh_TW.ts \
                i18n/l-fileinfo_zu.ts 

dotrans.path=$${L_SHAREDIR}/lumina-desktop/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/i18n/

desktop.files=lumina-fileinfo.desktop
desktop.path=$${L_SHAREDIR}/applications/

manpage.path=$${L_MANDIR}/man1/
manpage.extra="$${MAN_ZIP} $$PWD/lumina-fileinfo.1 > $(INSTALL_ROOT)$${L_MANDIR}/man1/lumina-fileinfo.1.gz"

INSTALLS += target desktop manpage

WITH_I18N{
  INSTALLS += dotrans
}
