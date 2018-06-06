//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2012-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================

#include "global-includes.h"
#include "LSession.h"

#define DEBUG 0

int main(int argc, char ** argv)
{
  qDebug() << "Starting lumina-desktop-unified...";
    if (argc > 1) {
      if (QString(argv[1]) == QString("--version")){
        qDebug() << LDesktopUtils::LuminaDesktopVersion();
        return 0;
      }
    }
    if(!QFile::exists(LOS::LuminaShare())){
      qDebug() << "Lumina does not appear to be installed correctly. Cannot find: " << LOS::LuminaShare();
      return 1;
    }
    //Setup any pre-QApplication initialization values
    LTHEME::LoadCustomEnvSettings();
    LXDG::setEnvironmentVars();
    setenv("DESKTOP_SESSION","Lumina",1);
    setenv("XDG_CURRENT_DESKTOP","Lumina",1);
    setenv("QT_NO_GLIB", "1", 1); //Disable the glib event loop within Qt at runtime (performance hit + bugs)
    setenv("QT_QPA_PLATFORMTHEME", "lthemeengine",1); //causes issues with Lumina themes - not many people have this by default...
    unsetenv("QT_AUTO_SCREEN_SCALE_FACTOR"); //need exact-pixel measurements (no fake scaling)

    //Startup the session
    if(DEBUG){ qDebug() << "Starting unified session"; }
    LSession a(argc, argv);
    if(!a.isPrimaryProcess()){ return 0; }
    QTime *timer=0;
    if(DEBUG){ timer = new QTime(); timer->start(); }
    if(DEBUG){ qDebug() << "Theme Init:" << timer->elapsed(); }
    /*LuminaThemeEngine theme(&a);
    QObject::connect(&theme, SIGNAL(updateIcons()), &a, SLOT(reloadIconTheme()) );*/
    if(DEBUG){ qDebug() << "Session Setup:" << timer->elapsed(); }
    QTimer::singleShot(0, &a, SLOT(setupSession()) );
    //theme.refresh();
    if(DEBUG){ qDebug() << "Exec Time:" << timer->elapsed(); delete timer;}
    int retCode = a.exec();
    qDebug() << "Finished Closing Down Unified Lumina";
    return retCode;
}
