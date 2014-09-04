#include <QTranslator>
#ifdef __linux
   // #include <QtSolutions/qtsingleapplication.h>
#else
  #include <qtsingleapplication.h>
#endif // #ifdef __linux
#include <QtGui/QApplication>
#include <QDebug>
#include <QFile>

#include "mainUI.h"

#ifndef PREFIX
#define PREFIX QString("/usr/local")
#endif

int main(int argc, char ** argv)
{
    #ifdef __linux
    QApplication a(argc, argv);
    #else
    QtSingleApplication a(argc, argv);
    if( a.isRunning() )
      return !(a.sendMessage("show"));
    #endif
    QTranslator translator;
    QLocale mylocale;
    QString langCode = mylocale.name();
    
    if ( ! QFile::exists(PREFIX + "/share/Lumina-DE/i18n/lumina-config_" + langCode + ".qm" ) )  langCode.truncate(langCode.indexOf("_"));
    translator.load( QString("lumina-config_") + langCode, PREFIX + "/share/i18n/Lumina-DE/" );
    a.installTranslator( &translator );
    qDebug() << "Locale:" << langCode;
    

    MainUI w;
    QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(slotSingleInstance()) );
    w.show();

    int retCode = a.exec();
    return retCode;
}
