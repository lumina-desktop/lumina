TEMPLATE	= app
LANGUAGE	= C++
QT += core gui widgets network
CONFIG	+= qt warn_on release

isEmpty(PREFIX) {
 PREFIX = /usr/local
}
TARGET  = lumina-terminal
target.path = $$DESTDIR$$PREFIX/bin

isEmpty(LIBPREFIX) {
 LIBPREFIX = $$PREFIX/lib
}

HEADERS	+= TrayIcon.h \
		TermWindow.h \
		TerminalWidget.h \
		TtyProcess.h
		
SOURCES	+= main.cpp \
		TrayIcon.cpp \
		TermWindow.cpp \
		TerminalWidget.cpp \
		TtyProcess.cpp


INCLUDEPATH += ../../libLumina $$PREFIX/include
LIBS += -L../../libLumina -L$$LIBPREFIX -lLuminaUtils -lc

isEmpty(QT5LIBDIR) {
 QT5LIBDIR = $$PREFIX/lib/qt5
}

LRELEASE = $$QT5LIBDIR/bin/lrelease


QMAKE_LIBDIR	= ../libLumina
DEPENDPATH	+= ../../libLumina

TRANSLATIONS =  i18n/lumina-terminal_af.ts \
                i18n/lumina-terminal_ar.ts \
                i18n/lumina-terminal_az.ts \
                i18n/lumina-terminal_bg.ts \
                i18n/lumina-terminal_bn.ts \
                i18n/lumina-terminal_bs.ts \
                i18n/lumina-terminal_ca.ts \
                i18n/lumina-terminal_cs.ts \
                i18n/lumina-terminal_cy.ts \
                i18n/lumina-terminal_da.ts \
                i18n/lumina-terminal_de.ts \
                i18n/lumina-terminal_el.ts \
                i18n/lumina-terminal_en_GB.ts \
                i18n/lumina-terminal_en_ZA.ts \
                i18n/lumina-terminal_es.ts \
                i18n/lumina-terminal_et.ts \
                i18n/lumina-terminal_eu.ts \
                i18n/lumina-terminal_fa.ts \
                i18n/lumina-terminal_fi.ts \
                i18n/lumina-terminal_fr.ts \
                i18n/lumina-terminal_fr_CA.ts \
                i18n/lumina-terminal_gl.ts \
                i18n/lumina-terminal_he.ts \
                i18n/lumina-terminal_hi.ts \
                i18n/lumina-terminal_hr.ts \
                i18n/lumina-terminal_hu.ts \
                i18n/lumina-terminal_id.ts \
                i18n/lumina-terminal_is.ts \
                i18n/lumina-terminal_it.ts \
                i18n/lumina-terminal_ja.ts \
                i18n/lumina-terminal_ka.ts \
                i18n/lumina-terminal_ko.ts \
                i18n/lumina-terminal_lt.ts \
                i18n/lumina-terminal_lv.ts \
                i18n/lumina-terminal_mk.ts \
                i18n/lumina-terminal_mn.ts \
                i18n/lumina-terminal_ms.ts \
                i18n/lumina-terminal_mt.ts \
                i18n/lumina-terminal_nb.ts \
                i18n/lumina-terminal_nl.ts \
                i18n/lumina-terminal_pa.ts \
                i18n/lumina-terminal_pl.ts \
                i18n/lumina-terminal_pt.ts \
                i18n/lumina-terminal_pt_BR.ts \
                i18n/lumina-terminal_ro.ts \
                i18n/lumina-terminal_ru.ts \
                i18n/lumina-terminal_sk.ts \
                i18n/lumina-terminal_sl.ts \
                i18n/lumina-terminal_sr.ts \
                i18n/lumina-terminal_sv.ts \
                i18n/lumina-terminal_sw.ts \
                i18n/lumina-terminal_ta.ts \
                i18n/lumina-terminal_tg.ts \
                i18n/lumina-terminal_th.ts \
                i18n/lumina-terminal_tr.ts \
                i18n/lumina-terminal_uk.ts \
                i18n/lumina-terminal_uz.ts \
                i18n/lumina-terminal_vi.ts \
                i18n/lumina-terminal_zh_CN.ts \
                i18n/lumina-terminal_zh_HK.ts \
                i18n/lumina-terminal_zh_TW.ts \
                i18n/lumina-terminal_zu.ts

dotrans.path=$$PREFIX/share/Lumina-DE/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$$PREFIX/share/Lumina-DE/i18n/

INSTALLS += target dotrans

NO_I18N{
  INSTALLS -= dotrans
}

