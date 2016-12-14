#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QStringList>

#include "MainUI.h"
#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LUtils.h>
#include <LuminaSingleApplication.h>

#include "BrowserWidget.h"

int main(int argc, char ** argv)
{
    LTHEME::LoadCustomEnvSettings();
    LSingleApplication a(argc, argv, "lumina-fm"); //loads translations inside constructor
      if( !a.isPrimaryProcess()){ return 0; }
    qDebug() << "Loaded QApplication";
    a.setApplicationName("Insight File Manager");
    //LuminaThemeEngine themes(&a);

    //Get the list of inputs for the initial load
    QStringList in = a.inputlist; //has fixes for relative paths and such
    if(in.isEmpty()){ in << QDir::homePath(); }
    //Start the UI
    MainUI w;
    QObject::connect(&a, SIGNAL(InputsAvailable(QStringList)), &w, SLOT(slotSingleInstance(QStringList)) );
    //QObject::connect(&themes, SIGNAL(updateIcons()), &w, SLOT(setupIcons()) );
    w.OpenDirs(in);
    w.show();

   // TESTING CODE FOR NEW BACKEND
    /*QMainWindow W;
      BrowserWidget B("",&W);
      W.setCentralWidget( &B);
      B.showDetails(false);
      B.changeDirectory(in.first());
    qDebug() << "Show Window";
    W.show();
  */

    int retCode = a.exec();
    return retCode;
}
