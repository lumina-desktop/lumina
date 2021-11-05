QT       += core gui widgets

TARGET = qsudo
target.path = $${DESTDIR}/usr/bin/
TEMPLATE = app

LIBS += -L/usr/local/lib

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

RESOURCES += qsudo.qrc

TRANSLATIONS =  i18n/qsudo_af.ts \
                i18n/qsudo_ar.ts \
                i18n/qsudo_az.ts \
                i18n/qsudo_bg.ts \
                i18n/qsudo_bn.ts \
                i18n/qsudo_bs.ts \
                i18n/qsudo_ca.ts \
                i18n/qsudo_cs.ts \
                i18n/qsudo_cy.ts \
                i18n/qsudo_da.ts \
                i18n/qsudo_de.ts \
                i18n/qsudo_el.ts \
                i18n/qsudo_es.ts \
                i18n/qsudo_en_GB.ts \
                i18n/qsudo_en_AU.ts \
                i18n/qsudo_en_ZA.ts \
                i18n/qsudo_et.ts \
                i18n/qsudo_eu.ts \
                i18n/qsudo_fa.ts \
                i18n/qsudo_fi.ts \
                i18n/qsudo_fr.ts \
                i18n/qsudo_fr_CA.ts \
                i18n/qsudo_fur.ts \
                i18n/qsudo_gl.ts \
                i18n/qsudo_he.ts \
                i18n/qsudo_hi.ts \
                i18n/qsudo_hr.ts \
                i18n/qsudo_hu.ts \
                i18n/qsudo_id.ts \
                i18n/qsudo_is.ts \
                i18n/qsudo_it.ts \
                i18n/qsudo_ja.ts \
                i18n/qsudo_ka.ts \
                i18n/qsudo_ko.ts \
                i18n/qsudo_lt.ts \
                i18n/qsudo_lv.ts \
                i18n/qsudo_mk.ts \
                i18n/qsudo_mn.ts \
                i18n/qsudo_ms.ts \
                i18n/qsudo_mt.ts \
                i18n/qsudo_nb.ts \
                i18n/qsudo_ne.ts \
                i18n/qsudo_nl.ts \
                i18n/qsudo_pa.ts \
                i18n/qsudo_pl.ts \
                i18n/qsudo_pt.ts \
                i18n/qsudo_pt_BR.ts \
                i18n/qsudo_ro.ts \
                i18n/qsudo_ru.ts \
                i18n/qsudo_sa.ts \
                i18n/qsudo_sk.ts \
                i18n/qsudo_sl.ts \
                i18n/qsudo_sr.ts \
                i18n/qsudo_sv.ts \
                i18n/qsudo_sw.ts \
                i18n/qsudo_ta.ts \
                i18n/qsudo_tg.ts \
                i18n/qsudo_th.ts \
                i18n/qsudo_tr.ts \
                i18n/qsudo_uk.ts \
                i18n/qsudo_ur.ts \
                i18n/qsudo_uz.ts \
                i18n/qsudo_vi.ts \
                i18n/qsudo_zh_CN.ts \
                i18n/qsudo_zh_HK.ts \
                i18n/qsudo_zh_TW.ts \
                i18n/qsudo_zu.ts

INSTALLS += target
