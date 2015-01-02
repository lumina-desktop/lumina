#include <QTranslator>

#include <QApplication>
#include <QDebug>
#include <QFile>

#include "mainUI.h"
#include <LuminaOS.h>
#include <LuminaUtils.h>
#include <LuminaThemes.h>
#include <LuminaSingleApplication.h>

int main(int argc, char ** argv)
{
    LSingleApplication a(argc, argv, "lumina-config"); //loads translations inside constructor
    if(!a.isPrimaryProcess()){ return 0; }
    
    LuminaThemeEngine theme(&a);

    MainUI w;
    QObject::connect(&a, SIGNAL(InputsAvailable(QStringList)), &w, SLOT(slotSingleInstance()) );
    QObject::connect(&theme, SIGNAL(updateIcons()), &w, SLOT(setupIcons()) );
    w.show();

    int retCode = a.exec();
    return retCode;
}
