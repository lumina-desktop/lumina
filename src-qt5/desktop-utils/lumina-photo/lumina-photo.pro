include("$${PWD}/../../OS-detect.pri")

QT       += core gui widgets svg printsupport


TARGET = lumina-photo
target.path = $${L_BINDIR}

TEMPLATE = app

#include all the special classes from the Lumina tree
include(../../core/libLumina/LUtils.pri) #includes LUtils
include(../../core/libLumina/LuminaXDG.pri)
include(../../core/libLumina/LuminaSingleApplication.pri)
include(../../core/libLumina/LuminaThemes.pri)

SOURCES +=  main.cpp\
            mainUI.cpp \
            PhotoView.cpp

HEADERS  += mainUI.h \
            PhotoView.h

FORMS += mainUI.ui

#RESOURCES += extra/resources.qrc

TRANSLATIONS =  i18n/l-photo_af.ts \
                i18n/l-photo_ar.ts \
                i18n/l-photo_az.ts \
                i18n/l-photo_bg.ts \
                i18n/l-photo_bn.ts \
                i18n/l-photo_bs.ts \
                i18n/l-photo_ca.ts \
                i18n/l-photo_cs.ts \
                i18n/l-photo_cy.ts \
                i18n/l-photo_da.ts \
                i18n/l-photo_de.ts \
                i18n/l-photo_el.ts \
                i18n/l-photo_en_GB.ts \
                i18n/l-photo_en_ZA.ts \
                i18n/l-photo_en_AU.ts \
                i18n/l-photo_es.ts \
                i18n/l-photo_et.ts \
                i18n/l-photo_eu.ts \
                i18n/l-photo_fa.ts \
                i18n/l-photo_fi.ts \
                i18n/l-photo_fr.ts \
                i18n/l-photo_fr_CA.ts \
                i18n/l-photo_gl.ts \
                i18n/l-photo_he.ts \
                i18n/l-photo_hi.ts \
                i18n/l-photo_hr.ts \
                i18n/l-photo_hu.ts \
                i18n/l-photo_id.ts \
                i18n/l-photo_is.ts \
                i18n/l-photo_it.ts \
                i18n/l-photo_ja.ts \
                i18n/l-photo_ka.ts \
                i18n/l-photo_ko.ts \
                i18n/l-photo_lt.ts \
                i18n/l-photo_lv.ts \
                i18n/l-photo_mk.ts \
                i18n/l-photo_mn.ts \
                i18n/l-photo_ms.ts \
                i18n/l-photo_mt.ts \
                i18n/l-photo_nb.ts \
                i18n/l-photo_nl.ts \
                i18n/l-photo_pa.ts \
                i18n/l-photo_pl.ts \
                i18n/l-photo_pt.ts \
                i18n/l-photo_pt_BR.ts \
                i18n/l-photo_ro.ts \
                i18n/l-photo_ru.ts \
                i18n/l-photo_sk.ts \
                i18n/l-photo_sl.ts \
                i18n/l-photo_sr.ts \
                i18n/l-photo_sv.ts \
                i18n/l-photo_sw.ts \
                i18n/l-photo_ta.ts \
                i18n/l-photo_tg.ts \
                i18n/l-photo_th.ts \
                i18n/l-photo_tr.ts \
                i18n/l-photo_uk.ts \
                i18n/l-photo_uz.ts \
                i18n/l-photo_vi.ts \
                i18n/l-photo_zh_CN.ts \
                i18n/l-photo_zh_HK.ts \
                i18n/l-photo_zh_TW.ts \
                i18n/l-photo_zu.ts

dotrans.path=$${L_SHAREDIR}/lumina-desktop/i18n/
dotrans.extra=cd $$PWD/i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/i18n/

desktop.files=lumina-photo.desktop
desktop.path=$${L_SHAREDIR}/applications/

#link.path=$${L_BINDIR}
#link.extra=ln -sf lumina-photo $(INSTALL_ROOT)$${L_BINDIR}/lplay

manpage.path=$${L_MANDIR}/man1/
manpage.extra="$${MAN_ZIP} $$PWD/lumina-photo.1 > $(INSTALL_ROOT)$${L_MANDIR}/man1/lumina-photo.1.gz"

INSTALLS += target desktop manpage

WITH_I18N{
  INSTALLS += dotrans
}
