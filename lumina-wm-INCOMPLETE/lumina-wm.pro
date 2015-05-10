
QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras 

TARGET = lumina-wm
isEmpty(PREFIX) {
 PREFIX = /usr/local
}
target.path = $$PREFIX/bin

isEmpty(LIBPREFIX) {
 LIBPREFIX = $$PREFIX/lib
}

LIBS     += -L../libLumina -L$$LIBPREFIX -lLuminaUtils -lXdamage -lX11 -lxcb -lxcb-damage
QMAKE_LIBDIR	= ../libLumina
DEPENDPATH	+= ../libLumina

TEMPLATE = app

isEmpty(QT5LIBDIR) {
 QT5LIBDIR = $$PREFIX/lib/qt5
}

linux: {
   LRELEASE = /usr/bin/lrelease
} else {
   LRELEASE = $$QT5LIBDIR/bin/lrelease
}

SOURCES += main.cpp \
		WMSession.cpp \
		LScreenSaver.cpp


HEADERS  += WMSession.h \
		LScreenSaver.h

FORMS    += 

INCLUDEPATH += ../libLumina $$PREFIX/include

TRANSLATIONS =  i18n/lumina-wm_af.ts \
                i18n/lumina-wm_ar.ts \
                i18n/lumina-wm_az.ts \
                i18n/lumina-wm_bg.ts \
                i18n/lumina-wm_bn.ts \
                i18n/lumina-wm_bs.ts \
                i18n/lumina-wm_ca.ts \
                i18n/lumina-wm_cs.ts \
                i18n/lumina-wm_cy.ts \
                i18n/lumina-wm_da.ts \
                i18n/lumina-wm_de.ts \
                i18n/lumina-wm_el.ts \
                i18n/lumina-wm_en_GB.ts \
                i18n/lumina-wm_en_ZA.ts \
                i18n/lumina-wm_es.ts \
                i18n/lumina-wm_et.ts \
                i18n/lumina-wm_eu.ts \
                i18n/lumina-wm_fa.ts \
                i18n/lumina-wm_fi.ts \
                i18n/lumina-wm_fr.ts \
                i18n/lumina-wm_fr_CA.ts \
                i18n/lumina-wm_gl.ts \
                i18n/lumina-wm_he.ts \
                i18n/lumina-wm_hi.ts \
                i18n/lumina-wm_hr.ts \
                i18n/lumina-wm_hu.ts \
                i18n/lumina-wm_id.ts \
                i18n/lumina-wm_is.ts \
                i18n/lumina-wm_it.ts \
                i18n/lumina-wm_ja.ts \
                i18n/lumina-wm_ka.ts \
                i18n/lumina-wm_ko.ts \
                i18n/lumina-wm_lt.ts \
                i18n/lumina-wm_lv.ts \
                i18n/lumina-wm_mk.ts \
                i18n/lumina-wm_mn.ts \
                i18n/lumina-wm_ms.ts \
                i18n/lumina-wm_mt.ts \
                i18n/lumina-wm_nb.ts \
                i18n/lumina-wm_nl.ts \
                i18n/lumina-wm_pa.ts \
                i18n/lumina-wm_pl.ts \
                i18n/lumina-wm_pt.ts \
                i18n/lumina-wm_pt_BR.ts \
                i18n/lumina-wm_ro.ts \
                i18n/lumina-wm_ru.ts \
                i18n/lumina-wm_sk.ts \
                i18n/lumina-wm_sl.ts \
                i18n/lumina-wm_sr.ts \
                i18n/lumina-wm_sv.ts \
                i18n/lumina-wm_sw.ts \
                i18n/lumina-wm_ta.ts \
                i18n/lumina-wm_tg.ts \
                i18n/lumina-wm_th.ts \
                i18n/lumina-wm_tr.ts \
                i18n/lumina-wm_uk.ts \
                i18n/lumina-wm_uz.ts \
                i18n/lumina-wm_vi.ts \
                i18n/lumina-wm_zh_CN.ts \
                i18n/lumina-wm_zh_HK.ts \
                i18n/lumina-wm_zh_TW.ts \
                i18n/lumina-wm_zu.ts

dotrans.path=$$PREFIX/share/Lumina-DE/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$$PREFIX/share/Lumina-DE/i18n/

INSTALLS += target dotrans
