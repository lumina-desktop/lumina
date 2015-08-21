#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QStringList>

#include "MainUI-old.h"
#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LuminaUtils.h>
#include <LuminaSingleApplication.h>

int main(int argc, char ** argv)
{
    LTHEME::LoadCustomEnvSettings();
    LSingleApplication a(argc, argv, "lumina-fm"); //loads translations inside constructor
      if( !a.isPrimaryProcess()){ return 0; }
    qDebug() << "Loaded QApplication";
    a.setApplicationName("Insight File Manager");
    LuminaThemeEngine themes(&a);

    //Get the list of inputs for the initial load
    QStringList in = a.inputlist; //has fixes for relative paths and such
    if(in.isEmpty()){ in << QDir::homePath(); }
    //Start the UI
    MainUI w;
    QObject::connect(&a, SIGNAL(InputsAvailable(QStringList)), &w, SLOT(slotSingleInstance(QStringList)) );
    QObject::connect(&themes, SIGNAL(updateIcons()), &w, SLOT(setupIcons()) );
    w.OpenDirs(in);
    w.show();

    int retCode = a.exec();
    return retCode;
}
