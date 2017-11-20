include("$${PWD}/../../OS-detect.pri")

QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras


TARGET = lumina-screenshot
target.path = $${L_BINDIR}

TEMPLATE = app

#include all the special classes from the Lumina tree
include(../../core/libLumina/LUtils.pri) #includes LUtils
include(../../core/libLumina/LuminaXDG.pri)
include(../../core/libLumina/LuminaSingleApplication.pri)
include(../../core/libLumina/LuminaThemes.pri)
include(../../core/libLumina/LuminaX11.pri)

SOURCES += main.cpp \
		MainUI.cpp \
		ImageEditor.cpp

HEADERS  += MainUI.h \
			ImageEditor.h

FORMS    += MainUI.ui

TRANSLATIONS =  i18n/l-screenshot_af.ts \
                i18n/l-screenshot_ar.ts \
                i18n/l-screenshot_az.ts \
                i18n/l-screenshot_bg.ts \
                i18n/l-screenshot_bn.ts \
                i18n/l-screenshot_bs.ts \
                i18n/l-screenshot_ca.ts \
                i18n/l-screenshot_cs.ts \
                i18n/l-screenshot_cy.ts \
                i18n/l-screenshot_da.ts \
                i18n/l-screenshot_de.ts \
                i18n/l-screenshot_el.ts \
                i18n/l-screenshot_en_GB.ts \
                i18n/l-screenshot_en_ZA.ts \
                i18n/l-screenshot_en_AU.ts \
                i18n/l-screenshot_es.ts \
                i18n/l-screenshot_et.ts \
                i18n/l-screenshot_eu.ts \
                i18n/l-screenshot_fa.ts \
                i18n/l-screenshot_fi.ts \
                i18n/l-screenshot_fr.ts \
                i18n/l-screenshot_fr_CA.ts \
                i18n/l-screenshot_gl.ts \
                i18n/l-screenshot_he.ts \
                i18n/l-screenshot_hi.ts \
                i18n/l-screenshot_hr.ts \
                i18n/l-screenshot_hu.ts \
                i18n/l-screenshot_id.ts \
                i18n/l-screenshot_is.ts \
                i18n/l-screenshot_it.ts \
                i18n/l-screenshot_ja.ts \
                i18n/l-screenshot_ka.ts \
                i18n/l-screenshot_ko.ts \
                i18n/l-screenshot_lt.ts \
                i18n/l-screenshot_lv.ts \
                i18n/l-screenshot_mk.ts \
                i18n/l-screenshot_mn.ts \
                i18n/l-screenshot_ms.ts \
                i18n/l-screenshot_mt.ts \
                i18n/l-screenshot_nb.ts \
                i18n/l-screenshot_nl.ts \
                i18n/l-screenshot_pa.ts \
                i18n/l-screenshot_pl.ts \
                i18n/l-screenshot_pt.ts \
                i18n/l-screenshot_pt_BR.ts \
                i18n/l-screenshot_ro.ts \
                i18n/l-screenshot_ru.ts \
                i18n/l-screenshot_sk.ts \
                i18n/l-screenshot_sl.ts \
                i18n/l-screenshot_sr.ts \
                i18n/l-screenshot_sv.ts \
                i18n/l-screenshot_sw.ts \
                i18n/l-screenshot_ta.ts \
                i18n/l-screenshot_tg.ts \
                i18n/l-screenshot_th.ts \
                i18n/l-screenshot_tr.ts \
                i18n/l-screenshot_uk.ts \
                i18n/l-screenshot_uz.ts \
                i18n/l-screenshot_vi.ts \
                i18n/l-screenshot_zh_CN.ts \
                i18n/l-screenshot_zh_HK.ts \
                i18n/l-screenshot_zh_TW.ts \
                i18n/l-screenshot_zu.ts

dotrans.path=$${L_SHAREDIR}/lumina-desktop/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/i18n/

desktop.files=lumina-screenshot.desktop
desktop.path=$${L_SHAREDIR}/applications/

manpage.path=$${L_MANDIR}/man8/
manpage.extra="$${MAN_ZIP} lumina-screenshot.8 > $(INSTALL_ROOT)$${L_MANDIR}/man8/lumina-screenshot.8.gz"

INSTALLS += target desktop manpage

WITH_I18N{
  INSTALLS += dotrans
}
