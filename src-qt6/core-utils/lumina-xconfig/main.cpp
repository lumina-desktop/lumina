#include <cstdio>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QStringList>

#include "MainUI.h"
#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LUtils.h>
#include <LuminaSingleApplication.h>

#include "ScreenSettings.h"
int main(int argc, char ** argv)
{
    bool CLIdone = false;
    for(int i=1; i<argc; i++){ //skip the first arg (app binary)
      if(QString(argv[i]) == "--reset-monitors"){
        RRSettings::ApplyPrevious();
        CLIdone = true;
        break;
      }else if(QString(argv[i]) == "--mirror-monitors"){
        RRSettings::MirrorAll();
        CLIdone = true;
        break;
      }else if(QString(argv[i]) == "--list-profiles"){
        //plain stdout, one profile per line (qDebug printed one quoted string with literal "\n" on stderr)
        QStringList profiles = RRSettings::savedProfiles();
        for(int p=0; p<profiles.length(); p++){ fprintf(stdout, "%s\n", profiles[p].toLocal8Bit().constData()); }
        CLIdone = true;
        break;
      }else if(QString(argv[i]) == "--apply-profile" && argc > (i+1) ){
        RRSettings::ApplyProfile( QString(argv[i+1]) );
        CLIdone = true;
        break;
      }
    }
    if(CLIdone){ return 0; }
   LTHEME::LoadCustomEnvSettings();
    LSingleApplication a(argc, argv, "lumina-xconfig"); //loads translations inside constructor
      if( !a.isPrimaryProcess()){ return 0; }
    //qDebug() << "Loaded QApplication";
    a.setApplicationName("Lumina Screen Configuration");
    //LuminaThemeEngine themes(&a);

    //Start the UI
    MainUI w;
    QObject::connect(&a, SIGNAL(InputsAvailable(QStringList)), &w, SLOT(slotSingleInstance()) );
    //QObject::connect(&themes, SIGNAL(updateIcons()), &w, SLOT(loadIcons()) );
    w.show();

    int retCode = a.exec();
    return retCode;
}
