include("$${PWD}/../../OS-detect.pri")

QT += core gui widgets

TARGET  = lumina-calculator
target.path = $${L_BINDIR}

#include all the special classes from the Lumina tree
include(../../core/libLumina/LUtils.pri) #includes LUtils
include(../../core/libLumina/LuminaXDG.pri)
#include(../../core/libLumina/LuminaSingleApplication.pri)
include(../../core/libLumina/LuminaThemes.pri)

HEADERS	+= mainUI.h \
		EqValidator.h
		
SOURCES	+= main.cpp \
			mainUI.cpp 

FORMS		+= mainUI.ui 

TRANSLATIONS =  i18n/l-calc_af.ts \
                i18n/l-calc_ar.ts \
                i18n/l-calc_az.ts \
                i18n/l-calc_bg.ts \
                i18n/l-calc_bn.ts \
                i18n/l-calc_bs.ts \
                i18n/l-calc_ca.ts \
                i18n/l-calc_cs.ts \
                i18n/l-calc_cy.ts \
                i18n/l-calc_da.ts \
                i18n/l-calc_de.ts \
                i18n/l-calc_el.ts \
                i18n/l-calc_en_GB.ts \
                i18n/l-calc_en_ZA.ts \
                i18n/l-calc_en_AU.ts \
                i18n/l-calc_es.ts \
                i18n/l-calc_et.ts \
                i18n/l-calc_eu.ts \
                i18n/l-calc_fa.ts \
                i18n/l-calc_fi.ts \
                i18n/l-calc_fr.ts \
                i18n/l-calc_fr_CA.ts \
                i18n/l-calc_gl.ts \
                i18n/l-calc_he.ts \
                i18n/l-calc_hi.ts \
                i18n/l-calc_hr.ts \
                i18n/l-calc_hu.ts \
                i18n/l-calc_id.ts \
                i18n/l-calc_is.ts \
                i18n/l-calc_it.ts \
                i18n/l-calc_ja.ts \
                i18n/l-calc_ka.ts \
                i18n/l-calc_ko.ts \
                i18n/l-calc_lt.ts \
                i18n/l-calc_lv.ts \
                i18n/l-calc_mk.ts \
                i18n/l-calc_mn.ts \
                i18n/l-calc_ms.ts \
                i18n/l-calc_mt.ts \
                i18n/l-calc_nb.ts \
                i18n/l-calc_nl.ts \
                i18n/l-calc_pa.ts \
                i18n/l-calc_pl.ts \
                i18n/l-calc_pt.ts \
                i18n/l-calc_pt_BR.ts \
                i18n/l-calc_ro.ts \
                i18n/l-calc_ru.ts \
                i18n/l-calc_sk.ts \
                i18n/l-calc_sl.ts \
                i18n/l-calc_sr.ts \
                i18n/l-calc_sv.ts \
                i18n/l-calc_sw.ts \
                i18n/l-calc_ta.ts \
                i18n/l-calc_tg.ts \
                i18n/l-calc_th.ts \
                i18n/l-calc_tr.ts \
                i18n/l-calc_uk.ts \
                i18n/l-calc_uz.ts \
                i18n/l-calc_vi.ts \
                i18n/l-calc_zh_CN.ts \
                i18n/l-calc_zh_HK.ts \
                i18n/l-calc_zh_TW.ts \
                i18n/l-calc_zu.ts

dotrans.path=$${L_SHAREDIR}/lumina-desktop/i18n/
dotrans.extra=cd $$PWD/i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/i18n/

desktop.files=lumina-calculator.desktop
desktop.path=$${L_SHAREDIR}/applications/

manpage.path=$${L_MANDIR}/man1/
manpage.extra="$${MAN_ZIP} $$PWD/lumina-calculator.1 > $(INSTALL_ROOT)$${L_MANDIR}/man1/lumina-calculator.1.gz"

INSTALLS += target desktop manpage

WITH_I18N{
  INSTALLS += dotrans
}
