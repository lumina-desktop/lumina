#include <QTranslator>
#ifdef __linux
   // #include <QtSolutions/qtsingleapplication.h>
#else
  #include <qtsingleapplication.h>
#endif // #ifdef __linux
#include <QtGui/QApplication>
#include <QDebug>
#include <QFile>

#include "MainUI.h"

#ifndef PREFIX
#define PREFIX QString("/usr/local")
#endif

int main(int argc, char ** argv)
{
    QStringList in;
    for(int i=1; i<argc; i++){ //skip the first arg (app binary)
      in << QString(argv[i]);
    }
    if(in.isEmpty()){ in << QDir::homePath(); }
    #ifdef __linux
    QApplication a(argc, argv);
    #else
    QtSingleApplication a(argc, argv);
    if( a.isRunning() ){
      return !(a.sendMessage(in.join("\n")));
    }
    #endif
    a.setApplicationName("Insight File Manager");
    QTranslator translator;
    QLocale mylocale;
    QString langCode = mylocale.name();
    
    if ( ! QFile::exists(PREFIX + "/share/Lumina-DE/i18n/lumina-fm_" + langCode + ".qm" ) )  langCode.truncate(langCode.indexOf("_"));
    translator.load( QString("lumina-fm_") + langCode, PREFIX + "/share/i18n/Lumina-DE/" );
    a.installTranslator( &translator );
    qDebug() << "Locale:" << langCode;
    
    MainUI w;
    QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(slotSingleInstance(const QString&)) );
    w.OpenDirs(in);
    w.show();

    int retCode = a.exec();
    return retCode;
}
