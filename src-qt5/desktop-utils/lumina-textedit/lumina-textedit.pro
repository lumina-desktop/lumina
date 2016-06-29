include("$${PWD}/../../OS-detect.pri")

QT += core gui widgets

TARGET  = lumina-textedit
target.path = $${L_BINDIR}

HEADERS	+= MainUI.h \
			PlainTextEditor.h \
			syntaxSupport.h \
			ColorDialog.h
		
SOURCES	+= main.cpp \
			MainUI.cpp \
			PlainTextEditor.cpp \
			syntaxSupport.cpp \
			ColorDialog.cpp

FORMS		+= MainUI.ui \
			ColorDialog.ui
LIBS += -lLuminaUtils


DEPENDPATH	+= ../../libLumina

TRANSLATIONS =  i18n/lumina-textedit_af.ts \
                i18n/lumina-textedit_ar.ts \
                i18n/lumina-textedit_az.ts \
                i18n/lumina-textedit_bg.ts \
                i18n/lumina-textedit_bn.ts \
                i18n/lumina-textedit_bs.ts \
                i18n/lumina-textedit_ca.ts \
                i18n/lumina-textedit_cs.ts \
                i18n/lumina-textedit_cy.ts \
                i18n/lumina-textedit_da.ts \
                i18n/lumina-textedit_de.ts \
                i18n/lumina-textedit_el.ts \
                i18n/lumina-textedit_en_GB.ts \
                i18n/lumina-textedit_en_ZA.ts \
                i18n/lumina-textedit_es.ts \
                i18n/lumina-textedit_et.ts \
                i18n/lumina-textedit_eu.ts \
                i18n/lumina-textedit_fa.ts \
                i18n/lumina-textedit_fi.ts \
                i18n/lumina-textedit_fr.ts \
                i18n/lumina-textedit_fr_CA.ts \
                i18n/lumina-textedit_gl.ts \
                i18n/lumina-textedit_he.ts \
                i18n/lumina-textedit_hi.ts \
                i18n/lumina-textedit_hr.ts \
                i18n/lumina-textedit_hu.ts \
                i18n/lumina-textedit_id.ts \
                i18n/lumina-textedit_is.ts \
                i18n/lumina-textedit_it.ts \
                i18n/lumina-textedit_ja.ts \
                i18n/lumina-textedit_ka.ts \
                i18n/lumina-textedit_ko.ts \
                i18n/lumina-textedit_lt.ts \
                i18n/lumina-textedit_lv.ts \
                i18n/lumina-textedit_mk.ts \
                i18n/lumina-textedit_mn.ts \
                i18n/lumina-textedit_ms.ts \
                i18n/lumina-textedit_mt.ts \
                i18n/lumina-textedit_nb.ts \
                i18n/lumina-textedit_nl.ts \
                i18n/lumina-textedit_pa.ts \
                i18n/lumina-textedit_pl.ts \
                i18n/lumina-textedit_pt.ts \
                i18n/lumina-textedit_pt_BR.ts \
                i18n/lumina-textedit_ro.ts \
                i18n/lumina-textedit_ru.ts \
                i18n/lumina-textedit_sk.ts \
                i18n/lumina-textedit_sl.ts \
                i18n/lumina-textedit_sr.ts \
                i18n/lumina-textedit_sv.ts \
                i18n/lumina-textedit_sw.ts \
                i18n/lumina-textedit_ta.ts \
                i18n/lumina-textedit_tg.ts \
                i18n/lumina-textedit_th.ts \
                i18n/lumina-textedit_tr.ts \
                i18n/lumina-textedit_uk.ts \
                i18n/lumina-textedit_uz.ts \
                i18n/lumina-textedit_vi.ts \
                i18n/lumina-textedit_zh_CN.ts \
                i18n/lumina-textedit_zh_HK.ts \
                i18n/lumina-textedit_zh_TW.ts \
                i18n/lumina-textedit_zu.ts

dotrans.path=$${L_SHAREDIR}/lumina-desktop/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/i18n/

desktop.files=lumina-textedit.desktop
desktop.path=$${L_SHAREDIR}/applications/

link.path=$${L_BINDIR}
link.extra=ln -sf $(INSTALL_ROOT)$${L_BINDIR}/lumina-textedit $(INSTALL_ROOT)$${L_BINDIR}/lte

INSTALLS += target desktop link

WITH_I18N{
  INSTALLS += dotrans
}
