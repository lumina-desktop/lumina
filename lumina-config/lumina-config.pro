
QT       += core gui

TARGET = lumina-config
target.path = /usr/local/bin

TEMPLATE = app

SOURCES += main.cpp \
	   mainUI.cpp \
	   LPlugins.cpp

HEADERS  += mainUI.h \
		LPlugins.h \
		KeyCatch.h

FORMS    += mainUI.ui \
		KeyCatch.ui


# RESOURCES+= lumina-config.qrc

INCLUDEPATH += ../libLumina /usr/local/include

linux-g++-64 {
  LIBS     += -L../libLumina -lLuminaUtils 
} else {
  LIBS     += -L../libLumina -lLuminaUtils -lQtSolutions_SingleApplication-head
}

TRANSLATIONS =  i18n/lumina-config_af.ts \
                i18n/lumina-config_ar.ts \
                i18n/lumina-config_az.ts \
                i18n/lumina-config_bg.ts \
                i18n/lumina-config_bn.ts \
                i18n/lumina-config_bs.ts \
                i18n/lumina-config_ca.ts \
                i18n/lumina-config_cs.ts \
                i18n/lumina-config_cy.ts \
                i18n/lumina-config_da.ts \
                i18n/lumina-config_de.ts \
                i18n/lumina-config_el.ts \
                i18n/lumina-config_en_GB.ts \
                i18n/lumina-config_en_ZA.ts \
                i18n/lumina-config_es.ts \
                i18n/lumina-config_et.ts \
                i18n/lumina-config_eu.ts \
                i18n/lumina-config_fa.ts \
                i18n/lumina-config_fi.ts \
                i18n/lumina-config_fr.ts \
                i18n/lumina-config_fr_CA.ts \
                i18n/lumina-config_gl.ts \
                i18n/lumina-config_he.ts \
                i18n/lumina-config_hi.ts \
                i18n/lumina-config_hr.ts \
                i18n/lumina-config_hu.ts \
                i18n/lumina-config_id.ts \
                i18n/lumina-config_is.ts \
                i18n/lumina-config_it.ts \
                i18n/lumina-config_ja.ts \
                i18n/lumina-config_ka.ts \
                i18n/lumina-config_ko.ts \
                i18n/lumina-config_lt.ts \
                i18n/lumina-config_lv.ts \
                i18n/lumina-config_mk.ts \
                i18n/lumina-config_mn.ts \
                i18n/lumina-config_ms.ts \
                i18n/lumina-config_mt.ts \
                i18n/lumina-config_nb.ts \
                i18n/lumina-config_nl.ts \
                i18n/lumina-config_pa.ts \
                i18n/lumina-config_pl.ts \
                i18n/lumina-config_pt.ts \
                i18n/lumina-config_pt_BR.ts \
                i18n/lumina-config_ro.ts \
                i18n/lumina-config_ru.ts \
                i18n/lumina-config_sk.ts \
                i18n/lumina-config_sl.ts \
                i18n/lumina-config_sr.ts \
                i18n/lumina-config_sv.ts \
                i18n/lumina-config_sw.ts \
                i18n/lumina-config_ta.ts \
                i18n/lumina-config_tg.ts \
                i18n/lumina-config_th.ts \
                i18n/lumina-config_tr.ts \
                i18n/lumina-config_uk.ts \
                i18n/lumina-config_uz.ts \
                i18n/lumina-config_vi.ts \
                i18n/lumina-config_zh_CN.ts \
                i18n/lumina-config_zh_HK.ts \
                i18n/lumina-config_zh_TW.ts \
                i18n/lumina-config_zu.ts

dotrans.path=/usr/local/share/Lumina-DE/i18n/
dotrans.extra=cd i18n && lrelease-qt4 -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)/usr/local/share/Lumina-DE/i18n/

INSTALLS += target dotrans
