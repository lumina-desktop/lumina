QT       += core gui widgets

TARGET = lumina-sudo
target.path = $${DESTDIR}/usr/bin/

include(../../core/libLumina/LuminaXDG.pri)

TEMPLATE = app

LIBS += -L/usr/local/lib

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

RESOURCES += lumina-sudo.qrc

TRANSLATIONS =  i18n/lum_udo_af.ts \
                i18n/lum_sudo_ar.ts \
                i18n/lum_sudo_az.ts \
                i18n/lum_sudo_bg.ts \
                i18n/lum_sudo_bn.ts \
                i18n/lum_sudo_bs.ts \
                i18n/lum_sudo_ca.ts \
                i18n/lum_sudo_cs.ts \
                i18n/lum_sudo_cy.ts \
                i18n/lum_sudo_da.ts \
                i18n/lum_sudo_de.ts \
                i18n/lum_sudo_el.ts \
                i18n/lum_sudo_es.ts \
                i18n/lum_sudo_en_GB.ts \
                i18n/lum_sudo_en_AU.ts \
                i18n/lum_sudo_en_ZA.ts \
                i18n/lum_sudo_et.ts \
                i18n/lum_sudo_eu.ts \
                i18n/lum_sudo_fa.ts \
                i18n/lum_sudo_fi.ts \
                i18n/lum_sudo_fr.ts \
                i18n/lum_sudo_fr_CA.ts \
                i18n/lum_sudo_fur.ts \
                i18n/lum_sudo_gl.ts \
                i18n/lum_sudo_he.ts \
                i18n/lum_sudo_hi.ts \
                i18n/lum_sudo_hr.ts \
                i18n/lum_sudo_hu.ts \
                i18n/lum_sudo_id.ts \
                i18n/lum_sudo_is.ts \
                i18n/lum_sudo_it.ts \
                i18n/lum_sudo_ja.ts \
                i18n/lum_sudo_ka.ts \
                i18n/lum_sudo_ko.ts \
                i18n/lum_sudo_lt.ts \
                i18n/lum_sudo_lv.ts \
                i18n/lum_sudo_mk.ts \
                i18n/lum_sudo_mn.ts \
                i18n/lum_sudo_ms.ts \
                i18n/lum_sudo_mt.ts \
                i18n/lum_sudo_nb.ts \
                i18n/lum_sudo_ne.ts \
                i18n/lum_sudo_nl.ts \
                i18n/lum_sudo_pa.ts \
                i18n/lum_sudo_pl.ts \
                i18n/lum_sudo_pt.ts \
                i18n/lum_sudo_pt_BR.ts \
                i18n/lum_sudo_ro.ts \
                i18n/lum_sudo_ru.ts \
                i18n/lum_sudo_sa.ts \
                i18n/lum_sudo_sk.ts \
                i18n/lum_sudo_sl.ts \
                i18n/lum_sudo_sr.ts \
                i18n/lum_sudo_sv.ts \
                i18n/lum_sudo_sw.ts \
                i18n/lum_sudo_ta.ts \
                i18n/lum_sudo_tg.ts \
                i18n/lum_sudo_th.ts \
                i18n/lum_sudo_tr.ts \
                i18n/lum_sudo_uk.ts \
                i18n/lum_sudo_ur.ts \
                i18n/lum_sudo_uz.ts \
                i18n/lum_sudo_vi.ts \
                i18n/lum_sudo_zh_CN.ts \
                i18n/lum_sudo_zh_HK.ts \
                i18n/lum_sudo_zh_TW.ts \
                i18n/lum_sudo_zu.ts

INSTALLS += target
