#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QStringList>

#include "MainUI.h"
#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LUtils.h>
#include <LuminaSingleApplication.h>

int main(int argc, char ** argv)
{
    //qDebug() << "Create Single Application";
    LTHEME::LoadCustomEnvSettings();
    LSingleApplication a(argc, argv, "lumina-info"); //loads translations inside constructor
      if( !a.isPrimaryProcess()){ return 0; }
    //qDebug() << "Set Application Name";
    a.setApplicationName("About Lumina");
    //qDebug() << "Start the UI";
    //Start the UI
    MainUI w;
    QObject::connect(&a, SIGNAL(InputsAvailable(QStringList)), &w, SLOT(slotSingleInstance()) );
    //qDebug() << "Show Normal";
    w.showNormal();

    int retCode = a.exec();
    return retCode;
}
