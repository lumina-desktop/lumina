QT       += core gui widgets network concurrent

TARGET = trident-networkmgr
target.path = /usr/bin

isEmpty(LRELEASE){ LRELEASE = $$[QT_INSTALL_BINS]/lrelease }

TEMPLATE = app

SOURCES += main.cpp \
		mainUI.cpp \
		network.cpp

HEADERS  += mainUI.h \
		network.h

FORMS    += mainUI.ui

#sudoconf.files = sudoers.d/trident-networkmgr
#sudoconf.path = /etc/sudoers.d

desktop.files = trident-networkmgr.desktop
desktop.path = /usr/share/applications

scripts.files = trident-enable-dhcpcdconf
scripts.path = /usr/bin

include(../common/SingleApp.pri)

TRANSLATIONS =  i18n/tri-nmgr_af.ts \
                i18n/tri-nmgr_ar.ts \
                i18n/tri-nmgr_az.ts \
                i18n/tri-nmgr_bg.ts \
                i18n/tri-nmgr_bn.ts \
                i18n/tri-nmgr_bs.ts \
                i18n/tri-nmgr_ca.ts \
                i18n/tri-nmgr_cs.ts \
                i18n/tri-nmgr_cy.ts \
                i18n/tri-nmgr_da.ts \
                i18n/tri-nmgr_de.ts \
                i18n/tri-nmgr_el.ts \
                i18n/tri-nmgr_en_GB.ts \
                i18n/tri-nmgr_en_ZA.ts \
                i18n/tri-nmgr_en_AU.ts \
                i18n/tri-nmgr_es.ts \
                i18n/tri-nmgr_et.ts \
                i18n/tri-nmgr_eu.ts \
                i18n/tri-nmgr_fa.ts \
                i18n/tri-nmgr_fi.ts \
                i18n/tri-nmgr_fr.ts \
                i18n/tri-nmgr_fr_CA.ts \
                i18n/tri-nmgr_gl.ts \
                i18n/tri-nmgr_he.ts \
                i18n/tri-nmgr_hi.ts \
                i18n/tri-nmgr_hr.ts \
                i18n/tri-nmgr_hu.ts \
                i18n/tri-nmgr_id.ts \
                i18n/tri-nmgr_is.ts \
                i18n/tri-nmgr_it.ts \
                i18n/tri-nmgr_ja.ts \
                i18n/tri-nmgr_ka.ts \
                i18n/tri-nmgr_ko.ts \
                i18n/tri-nmgr_lt.ts \
                i18n/tri-nmgr_lv.ts \
                i18n/tri-nmgr_mk.ts \
                i18n/tri-nmgr_mn.ts \
                i18n/tri-nmgr_ms.ts \
                i18n/tri-nmgr_mt.ts \
                i18n/tri-nmgr_nb.ts \
                i18n/tri-nmgr_nl.ts \
                i18n/tri-nmgr_pa.ts \
                i18n/tri-nmgr_pl.ts \
                i18n/tri-nmgr_pt.ts \
                i18n/tri-nmgr_pt_BR.ts \
                i18n/tri-nmgr_ro.ts \
                i18n/tri-nmgr_ru.ts \
                i18n/tri-nmgr_sk.ts \
                i18n/tri-nmgr_sl.ts \
                i18n/tri-nmgr_sr.ts \
                i18n/tri-nmgr_sv.ts \
                i18n/tri-nmgr_sw.ts \
                i18n/tri-nmgr_ta.ts \
                i18n/tri-nmgr_tg.ts \
                i18n/tri-nmgr_th.ts \
                i18n/tri-nmgr_tr.ts \
                i18n/tri-nmgr_uk.ts \
                i18n/tri-nmgr_uz.ts \
                i18n/tri-nmgr_vi.ts \
                i18n/tri-nmgr_zh_CN.ts \
                i18n/tri-nmgr_zh_HK.ts \
                i18n/tri-nmgr_zh_TW.ts \
                i18n/tri-nmgr_zu.ts

dotrans.path=/usr/share/trident-networkmgr/i18n/
dotrans.extra=cd $$PWD/i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)/usr/share/trident-networkmgr/i18n/

#Some conf to redirect intermediate stuff in separate dirs
UI_DIR=./.build/ui/
MOC_DIR=./.build/moc/
OBJECTS_DIR=./.build/obj
RCC_DIR=./.build/rcc
QMAKE_DISTCLEAN += -r ./.build

INSTALLS += target dotrans scripts desktop
