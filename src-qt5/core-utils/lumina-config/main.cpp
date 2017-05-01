#include <QTranslator>

#include <QApplication>
#include <QDebug>
#include <QFile>


#include "mainWindow.h"
#include <LuminaOS.h>
#include <LUtils.h>
#include <LuminaThemes.h>
#include <LuminaSingleApplication.h>
#include <LuminaXDG.h>

XDGDesktopList *APPSLIST = 0;

int main(int argc, char ** argv)
{
    LTHEME::LoadCustomEnvSettings();
    LSingleApplication a(argc, argv, "lumina-config"); //loads translations inside constructor
    if(!a.isPrimaryProcess()){ return 0; } 
    //LuminaThemeEngine theme(&a);
    QStringList args;
    for(int i=1; i<argc; i++){ args << QString(argv[i]); }
    mainWindow w;
    QObject::connect(&a, SIGNAL(InputsAvailable(QStringList)), &w, SLOT(slotSingleInstance(QStringList)) );
    w.slotSingleInstance(args);
    w.show();

    int retCode = a.exec();
    return retCode;
}
