#include <QTranslator>

#ifdef __FreeBSD__
  #include <qtsingleapplication.h>
#endif
#include <QtGui/QApplication>
#include <QDebug>
#include <QFile>

#include "mainUI.h"
#include <LuminaOS.h>

int main(int argc, char ** argv)
{
    #ifndef __FreeBSD__
    QApplication a(argc, argv);
    #else
    QtSingleApplication a(argc, argv);
    if( a.isRunning() )
      return !(a.sendMessage("show"));
    #endif
    QTranslator translator;
    QLocale mylocale;
    QString langCode = mylocale.name();

    if ( ! QFile::exists(LOS::LuminaShare()+"i18n/lumina-config_" + langCode + ".qm" ) )  langCode.truncate(langCode.indexOf("_"));
    translator.load( QString("lumina-config_") + langCode, LOS::LuminaShare()+"i18n/" );
    a.installTranslator( &translator );
    qDebug() << "Locale:" << langCode;


    MainUI w;
    QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(slotSingleInstance()) );
    w.show();

    int retCode = a.exec();
    return retCode;
}
