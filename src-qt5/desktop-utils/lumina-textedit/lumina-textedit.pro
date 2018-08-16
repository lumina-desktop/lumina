include("$${PWD}/../../OS-detect.pri")

QT += core gui widgets printsupport

TARGET  = lumina-textedit
target.path = $${L_BINDIR}

#include all the special classes from the Lumina tree
include(../../core/libLumina/LUtils.pri) #includes LUtils
include(../../core/libLumina/LuminaXDG.pri)
#include(../../core/libLumina/LuminaSingleApplication.pri)
include(../../core/libLumina/LuminaThemes.pri)

HEADERS	+= MainUI.h \
			PlainTextEditor.h \
			syntaxSupport.h \
			ColorDialog.h \
			DnDTabBar.h \
      Word.h

SOURCES	+= main.cpp \
			MainUI.cpp \
			PlainTextEditor.cpp \
			syntaxSupport.cpp \
			ColorDialog.cpp

FORMS		+= MainUI.ui \
			ColorDialog.ui

#LIBS  +=  -lhunspell-1.6

TRANSLATIONS =  i18n/l-te_af.ts \
                i18n/l-te_ar.ts \
                i18n/l-te_az.ts \
                i18n/l-te_bg.ts \
                i18n/l-te_bn.ts \
                i18n/l-te_bs.ts \
                i18n/l-te_ca.ts \
                i18n/l-te_cs.ts \
                i18n/l-te_cy.ts \
                i18n/l-te_da.ts \
                i18n/l-te_de.ts \
                i18n/l-te_el.ts \
                i18n/l-te_en_GB.ts \
                i18n/l-te_en_ZA.ts \
                i18n/l-te_en_AU.ts \
                i18n/l-te_es.ts \
                i18n/l-te_et.ts \
                i18n/l-te_eu.ts \
                i18n/l-te_fa.ts \
                i18n/l-te_fi.ts \
                i18n/l-te_fr.ts \
                i18n/l-te_fr_CA.ts \
                i18n/l-te_gl.ts \
                i18n/l-te_he.ts \
                i18n/l-te_hi.ts \
                i18n/l-te_hr.ts \
                i18n/l-te_hu.ts \
                i18n/l-te_id.ts \
                i18n/l-te_is.ts \
                i18n/l-te_it.ts \
                i18n/l-te_ja.ts \
                i18n/l-te_ka.ts \
                i18n/l-te_ko.ts \
                i18n/l-te_lt.ts \
                i18n/l-te_lv.ts \
                i18n/l-te_mk.ts \
                i18n/l-te_mn.ts \
                i18n/l-te_ms.ts \
                i18n/l-te_mt.ts \
                i18n/l-te_nb.ts \
                i18n/l-te_nl.ts \
                i18n/l-te_pa.ts \
                i18n/l-te_pl.ts \
                i18n/l-te_pt.ts \
                i18n/l-te_pt_BR.ts \
                i18n/l-te_ro.ts \
                i18n/l-te_ru.ts \
                i18n/l-te_sk.ts \
                i18n/l-te_sl.ts \
                i18n/l-te_sr.ts \
                i18n/l-te_sv.ts \
                i18n/l-te_sw.ts \
                i18n/l-te_ta.ts \
                i18n/l-te_tg.ts \
                i18n/l-te_th.ts \
                i18n/l-te_tr.ts \
                i18n/l-te_uk.ts \
                i18n/l-te_uz.ts \
                i18n/l-te_vi.ts \
                i18n/l-te_zh_CN.ts \
                i18n/l-te_zh_HK.ts \
                i18n/l-te_zh_TW.ts \
                i18n/l-te_zu.ts

dotrans.path=$${L_SHAREDIR}/lumina-desktop/i18n/
dotrans.extra=cd $$PWD/i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/i18n/

desktop.files=lumina-textedit.desktop
desktop.path=$${L_SHAREDIR}/applications/

link.path=$${L_BINDIR}
link.extra=ln -sf lumina-textedit $(INSTALL_ROOT)$${L_BINDIR}/lte

syntax.path=$${L_SHAREDIR}/lumina-desktop/syntax_rules
syntax.files=syntax_rules/*

manpage.path=$${L_MANDIR}/man1/
manpage.extra="$${MAN_ZIP} $$PWD/lumina-textedit.1 > $(INSTALL_ROOT)$${L_MANDIR}/man1/lumina-textedit.1.gz"

INSTALLS += target desktop link syntax manpage

WITH_I18N{
  INSTALLS += dotrans
}
