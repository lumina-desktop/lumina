#include <QTranslator>
#ifdef __FreeBSD__
  #include <qtsingleapplication.h>
#endif
#include <QtGui/QApplication>
#include <QDebug>
#include <QFile>
#include <QTextCodec>

#include "MainUI.h"
#include <LuminaOS.h>
//#include <LuminaThemes.h>

int main(int argc, char ** argv)
{
    QStringList in;
    for(int i=1; i<argc; i++){ //skip the first arg (app binary)
      in << QString(argv[i]);
    }
    if(in.isEmpty()){ in << QDir::homePath(); }
    #ifdef __FreeBSD__
    QtSingleApplication a(argc, argv);
    if( a.isRunning() ){
      return !(a.sendMessage(in.join("\n")));
    }
    #else
    QApplication a(argc, argv);
    #endif
    a.setApplicationName("Insight File Manager");
    //LuminaThemeEngine themes(&a);
    //qDebug() << "StyleSheet:\n" << a.styleSheet();
    //Load current Locale
    QTranslator translator;
    QLocale mylocale;
    QString langCode = mylocale.name();

    if ( ! QFile::exists(LOS::LuminaShare()+"i18n/lumina-fm_" + langCode + ".qm" ) )  langCode.truncate(langCode.indexOf("_"));
    translator.load( QString("lumina-fm_") + langCode, LOS::LuminaShare()+"i18n/" );
    a.installTranslator( &translator );
    qDebug() << "Locale:" << langCode;

    //Load current encoding for this locale
    QTextCodec::setCodecForTr( QTextCodec::codecForLocale() ); //make sure to use the same codec
    qDebug() << "Locale Encoding:" << QTextCodec::codecForLocale()->name();

    MainUI w;
    QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(slotSingleInstance(const QString&)) );
    w.OpenDirs(in);
    w.show();

    int retCode = a.exec();
    return retCode;
}
