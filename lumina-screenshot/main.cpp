#include <QTranslator>
#include <QtGui/QApplication>
#include <QDebug>
#include <QFile>

#include "MainUI.h"

#ifndef PREFIX
#define PREFIX QString("/usr/local")
#endif

int main(int argc, char ** argv)
{
    QApplication a(argc, argv);
    a.setApplicationName("Take Screenshot");
    QTranslator translator;
    QLocale mylocale;
    QString langCode = mylocale.name();
    
    if ( ! QFile::exists(PREFIX + "/share/Lumina-DE/i18n/lumina-screenshot_" + langCode + ".qm" ) )  langCode.truncate(langCode.indexOf("_"));
    translator.load( QString("lumina-screenshot_") + langCode, PREFIX + "/share/i18n/Lumina-DE/" );
    a.installTranslator( &translator );
    qDebug() << "Locale:" << langCode;
    
    MainUI w;
    w.show();

    return a.exec();
}
