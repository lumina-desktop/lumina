include("$${PWD}/../../OS-detect.pri")

QT += core gui widgets network

TARGET  = lumina-terminal
target.path = $${L_BINDIR}

#include all the special classes from the Lumina tree
include(../../core/libLumina/LUtils.pri) #includes LUtils
include(../../core/libLumina/LuminaXDG.pri)
include(../../core/libLumina/LuminaSingleApplication.pri)
include(../../core/libLumina/LuminaThemes.pri)

HEADERS	+= TrayIcon.h \
		TermWindow.h \
		TerminalWidget.h \
		TtyProcess.h
		
SOURCES	+= main.cpp \
		TrayIcon.cpp \
		TermWindow.cpp \
		TerminalWidget.cpp \
		TtyProcess.cpp


LIBS += -lutil


TRANSLATIONS =  i18n/l-terminal_af.ts \
                i18n/l-terminal_ar.ts \
                i18n/l-terminal_az.ts \
                i18n/l-terminal_bg.ts \
                i18n/l-terminal_bn.ts \
                i18n/l-terminal_bs.ts \
                i18n/l-terminal_ca.ts \
                i18n/l-terminal_cs.ts \
                i18n/l-terminal_cy.ts \
                i18n/l-terminal_da.ts \
                i18n/l-terminal_de.ts \
                i18n/l-terminal_el.ts \
                i18n/l-terminal_en_GB.ts \
                i18n/l-terminal_en_ZA.ts \
                i18n/l-terminal_es.ts \
                i18n/l-terminal_et.ts \
                i18n/l-terminal_eu.ts \
                i18n/l-terminal_fa.ts \
                i18n/l-terminal_fi.ts \
                i18n/l-terminal_fr.ts \
                i18n/l-terminal_fr_CA.ts \
                i18n/l-terminal_gl.ts \
                i18n/l-terminal_he.ts \
                i18n/l-terminal_hi.ts \
                i18n/l-terminal_hr.ts \
                i18n/l-terminal_hu.ts \
                i18n/l-terminal_id.ts \
                i18n/l-terminal_is.ts \
                i18n/l-terminal_it.ts \
                i18n/l-terminal_ja.ts \
                i18n/l-terminal_ka.ts \
                i18n/l-terminal_ko.ts \
                i18n/l-terminal_lt.ts \
                i18n/l-terminal_lv.ts \
                i18n/l-terminal_mk.ts \
                i18n/l-terminal_mn.ts \
                i18n/l-terminal_ms.ts \
                i18n/l-terminal_mt.ts \
                i18n/l-terminal_nb.ts \
                i18n/l-terminal_nl.ts \
                i18n/l-terminal_pa.ts \
                i18n/l-terminal_pl.ts \
                i18n/l-terminal_pt.ts \
                i18n/l-terminal_pt_BR.ts \
                i18n/l-terminal_ro.ts \
                i18n/l-terminal_ru.ts \
                i18n/l-terminal_sk.ts \
                i18n/l-terminal_sl.ts \
                i18n/l-terminal_sr.ts \
                i18n/l-terminal_sv.ts \
                i18n/l-terminal_sw.ts \
                i18n/l-terminal_ta.ts \
                i18n/l-terminal_tg.ts \
                i18n/l-terminal_th.ts \
                i18n/l-terminal_tr.ts \
                i18n/l-terminal_uk.ts \
                i18n/l-terminal_uz.ts \
                i18n/l-terminal_vi.ts \
                i18n/l-terminal_zh_CN.ts \
                i18n/l-terminal_zh_HK.ts \
                i18n/l-terminal_zh_TW.ts \
                i18n/l-terminal_zu.ts

dotrans.path=$${L_SHAREDIR}/lumina-desktop/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/i18n/

desktop.files=lumina-terminal.desktop
desktop.path=$${L_SHAREDIR}/applications/

INSTALLS += target desktop

WITH_I18N{
  INSTALLS += dotrans
}
