#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QStringList>

#include "MainUI.h"
#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LuminaUtils.h>
#include <LuminaSingleApplication.h>

int main(int argc, char ** argv)
{

    LSingleApplication a(argc, argv, "lumina-info"); //loads translations inside constructor
      if( !a.isPrimaryProcess()){ return 0; }
    a.setApplicationName("About Lumina-DE");
    LuminaThemeEngine themes(&a);

    //Start the UI
    MainUI w;
    QObject::connect(&a, SIGNAL(InputsAvailable(QStringList)), &w, SLOT(slotSingleInstance()) );
    w.show();

    int retCode = a.exec();
    return retCode;
}
