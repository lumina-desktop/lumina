#include <QTranslator>
#include <QApplication>
#include <QDebug>
#include <QFile>

#include "MainUI.h"
#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LuminaUtils.h>

int main(int argc, char ** argv)
{
    QApplication a(argc, argv);
    LuminaThemeEngine theme(&a);
    a.setApplicationName("Take Screenshot");
    LUtils::LoadTranslation(&a, "lumina-screenshot");
    /*QTranslator translator;
    QLocale mylocale;
    QString langCode = mylocale.name();

    if ( ! QFile::exists(LOS::LuminaShare()+"i18n/lumina-screenshot_" + langCode + ".qm" ) )  langCode.truncate(langCode.indexOf("_"));
    translator.load( QString("lumina-screenshot_") + langCode, LOS::LuminaShare()+"i18n/" );
    a.installTranslator( &translator );
    qDebug() << "Locale:" << langCode;*/

    MainUI w;
    QObject::connect(&theme,SIGNAL(updateIcons()), &w, SLOT(setupIcons()) );
    w.show();

    return a.exec();
}
