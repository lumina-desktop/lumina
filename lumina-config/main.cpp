#include <QTranslator>

#include <QApplication>
#include <QDebug>
#include <QFile>

#include "mainUI.h"
#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LuminaSingleApplication.h>

int main(int argc, char ** argv)
{
    LSingleApplication a(argc, argv);
    if(!a.isPrimaryProcess()){ return 0; }
    
    LuminaThemeEngine theme(&a);
    QTranslator translator;
    QLocale mylocale;
    QString langCode = mylocale.name();

    if ( ! QFile::exists(LOS::LuminaShare()+"i18n/lumina-config_" + langCode + ".qm" ) )  langCode.truncate(langCode.indexOf("_"));
    translator.load( QString("lumina-config_") + langCode, LOS::LuminaShare()+"i18n/" );
    a.installTranslator( &translator );
    qDebug() << "Locale:" << langCode;


    MainUI w;
    QObject::connect(&a, SIGNAL(InputsAvailable(QStringList)), &w, SLOT(slotSingleInstance()) );
    QObject::connect(&theme, SIGNAL(updateIcons()), &w, SLOT(setupIcons()) );
    w.show();

    int retCode = a.exec();
    return retCode;
}
