include("$${PWD}/../../OS-detect.pri")

QT += core gui widgets

TARGET  = lumina-archiver
target.path = $${L_BINDIR}

#include all the special classes from the Lumina tree
include(../../core/libLumina/LUtils.pri) #includes LUtils
include(../../core/libLumina/LuminaXDG.pri)
include(../../core/libLumina/LuminaThemes.pri)

HEADERS	+= MainUI.h \
	TarBackend.h \
	imgDialog.h
		
SOURCES	+= main.cpp \
	MainUI.cpp \
	TarBackend.cpp \
	imgDialog.cpp

FORMS		+= MainUI.ui \
	imgDialog.ui

TRANSLATIONS =  i18n/l-archiver_af.ts \
                i18n/l-archiver_ar.ts \
                i18n/l-archiver_az.ts \
                i18n/l-archiver_bg.ts \
                i18n/l-archiver_bn.ts \
                i18n/l-archiver_bs.ts \
                i18n/l-archiver_ca.ts \
                i18n/l-archiver_cs.ts \
                i18n/l-archiver_cy.ts \
                i18n/l-archiver_da.ts \
                i18n/l-archiver_de.ts \
                i18n/l-archiver_el.ts \
                i18n/l-archiver_en_GB.ts \
                i18n/l-archiver_en_ZA.ts \
                i18n/l-archiver_en_AU.ts \
                i18n/l-archiver_es.ts \
                i18n/l-archiver_et.ts \
                i18n/l-archiver_eu.ts \
                i18n/l-archiver_fa.ts \
                i18n/l-archiver_fi.ts \
                i18n/l-archiver_fr.ts \
                i18n/l-archiver_fr_CA.ts \
                i18n/l-archiver_gl.ts \
                i18n/l-archiver_he.ts \
                i18n/l-archiver_hi.ts \
                i18n/l-archiver_hr.ts \
                i18n/l-archiver_hu.ts \
                i18n/l-archiver_id.ts \
                i18n/l-archiver_is.ts \
                i18n/l-archiver_it.ts \
                i18n/l-archiver_ja.ts \
                i18n/l-archiver_ka.ts \
                i18n/l-archiver_ko.ts \
                i18n/l-archiver_lt.ts \
                i18n/l-archiver_lv.ts \
                i18n/l-archiver_mk.ts \
                i18n/l-archiver_mn.ts \
                i18n/l-archiver_ms.ts \
                i18n/l-archiver_mt.ts \
                i18n/l-archiver_nb.ts \
                i18n/l-archiver_nl.ts \
                i18n/l-archiver_pa.ts \
                i18n/l-archiver_pl.ts \
                i18n/l-archiver_pt.ts \
                i18n/l-archiver_pt_BR.ts \
                i18n/l-archiver_ro.ts \
                i18n/l-archiver_ru.ts \
                i18n/l-archiver_sk.ts \
                i18n/l-archiver_sl.ts \
                i18n/l-archiver_sr.ts \
                i18n/l-archiver_sv.ts \
                i18n/l-archiver_sw.ts \
                i18n/l-archiver_ta.ts \
                i18n/l-archiver_tg.ts \
                i18n/l-archiver_th.ts \
                i18n/l-archiver_tr.ts \
                i18n/l-archiver_uk.ts \
                i18n/l-archiver_uz.ts \
                i18n/l-archiver_vi.ts \
                i18n/l-archiver_zh_CN.ts \
                i18n/l-archiver_zh_HK.ts \
                i18n/l-archiver_zh_TW.ts \
                i18n/l-archiver_zu.ts

dotrans.path=$${L_SHAREDIR}/lumina-desktop/i18n/
dotrans.extra=cd $$PWD/i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/i18n/

desktop.files=lumina-archiver.desktop
desktop.path=$${L_SHAREDIR}/applications/

#link.path=$${L_BINDIR}
#link.extra=ln -sf lumina-archiver $(INSTALL_ROOT)$${L_BINDIR}/lpac

manpage.path=$${L_MANDIR}/man1/
manpage.extra="$${MAN_ZIP} $$PWD/lumina-archiver.1 > $(INSTALL_ROOT)$${L_MANDIR}/man1/lumina-archiver.1.gz"

INSTALLS += target desktop manpage

WITH_I18N{
  INSTALLS += dotrans
}
