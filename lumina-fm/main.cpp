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
    /*QStringList in;
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
    if(in.isEmpty()){ in << QDir::homePath(); }*/

    LSingleApplication a(argc, argv);
      if( !a.isPrimaryProcess()){ return 0; }
    qDebug() << "Loaded QApplication";
    a.setApplicationName("Insight File Manager");
    LuminaThemeEngine themes(&a);
    LUtils::LoadTranslation(&a, "lumina-fm");

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
