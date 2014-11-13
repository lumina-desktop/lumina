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
#include <LuminaThemes.h>

int main(int argc, char ** argv)
{
    QStringList in;
    for(int i=1; i<argc; i++){ //skip the first arg (app binary)
      QString path = argv[i];
      if(path=="."){
	//Insert the current working directory
	in << QDir::currentPath();
      }else{
	if(!path.startsWith("/")){ path.prepend(QDir::currentPath()+"/"); }
        in << path;
      }
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
    LuminaThemeEngine themes(&a);
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
    QObject::connect(&themes, SIGNAL(updateIcons()), &w, SLOT(setupIcons()) );
    w.OpenDirs(in);
    w.show();

    int retCode = a.exec();
    return retCode;
}
