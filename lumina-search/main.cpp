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
    //qDebug() << "Init App...";
    QApplication a(argc, argv);
    //qDebug() << "Init Theme Engine...";
    LuminaThemeEngine theme(&a);
    //qDebug() << "Load Translations...";
    a.setApplicationName("Search for...");
    LUtils::LoadTranslation(&a, "lumina-search");
    /*QTranslator translator;
    QLocale mylocale;
    QString langCode = mylocale.name();

    if ( ! QFile::exists(LOS::LuminaShare()+"i18n/lumina-search_" + langCode + ".qm" ) )  langCode.truncate(langCode.indexOf("_"));
    translator.load( QString("lumina-search_") + langCode, LOS::LuminaShare()+"i18n/" );
    a.installTranslator( &translator );
    qDebug() << "Locale:" << langCode;*/

    MainUI w;
    QObject::connect(&theme,SIGNAL(updateIcons()), &w, SLOT(setupIcons()) );
    w.show();

    return a.exec();
}
