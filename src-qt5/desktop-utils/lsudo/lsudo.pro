QT       += core gui widgets

TARGET = lsudo
target.path = $${DESTDIR}/usr/bin/
TEMPLATE = app

LIBS += -L/usr/local/lib

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

RESOURCES += lsudo.qrc

TRANSLATIONS =  i18n/lsudo_af.ts \
                i18n/lsudo_ar.ts \
                i18n/lsudo_az.ts \
                i18n/lsudo_bg.ts \
                i18n/lsudo_bn.ts \
                i18n/lsudo_bs.ts \
                i18n/lsudo_ca.ts \
                i18n/lsudo_cs.ts \
                i18n/lsudo_cy.ts \
                i18n/lsudo_da.ts \
                i18n/lsudo_de.ts \
                i18n/lsudo_el.ts \
                i18n/lsudo_es.ts \
                i18n/lsudo_en_GB.ts \
                i18n/lsudo_en_AU.ts \
                i18n/lsudo_en_ZA.ts \
                i18n/lsudo_et.ts \
                i18n/lsudo_eu.ts \
                i18n/lsudo_fa.ts \
                i18n/lsudo_fi.ts \
                i18n/lsudo_fr.ts \
                i18n/lsudo_fr_CA.ts \
                i18n/lsudo_fur.ts \
                i18n/lsudo_gl.ts \
                i18n/lsudo_he.ts \
                i18n/lsudo_hi.ts \
                i18n/lsudo_hr.ts \
                i18n/lsudo_hu.ts \
                i18n/lsudo_id.ts \
                i18n/lsudo_is.ts \
                i18n/lsudo_it.ts \
                i18n/lsudo_ja.ts \
                i18n/lsudo_ka.ts \
                i18n/lsudo_ko.ts \
                i18n/lsudo_lt.ts \
                i18n/lsudo_lv.ts \
                i18n/lsudo_mk.ts \
                i18n/lsudo_mn.ts \
                i18n/lsudo_ms.ts \
                i18n/lsudo_mt.ts \
                i18n/lsudo_nb.ts \
                i18n/lsudo_ne.ts \
                i18n/lsudo_nl.ts \
                i18n/lsudo_pa.ts \
                i18n/lsudo_pl.ts \
                i18n/lsudo_pt.ts \
                i18n/lsudo_pt_BR.ts \
                i18n/lsudo_ro.ts \
                i18n/lsudo_ru.ts \
                i18n/lsudo_sa.ts \
                i18n/lsudo_sk.ts \
                i18n/lsudo_sl.ts \
                i18n/lsudo_sr.ts \
                i18n/lsudo_sv.ts \
                i18n/lsudo_sw.ts \
                i18n/lsudo_ta.ts \
                i18n/lsudo_tg.ts \
                i18n/lsudo_th.ts \
                i18n/lsudo_tr.ts \
                i18n/lsudo_uk.ts \
                i18n/lsudo_ur.ts \
                i18n/lsudo_uz.ts \
                i18n/lsudo_vi.ts \
                i18n/lsudo_zh_CN.ts \
                i18n/lsudo_zh_HK.ts \
                i18n/lsudo_zh_TW.ts \
                i18n/lsudo_zu.ts

INSTALLS += target
