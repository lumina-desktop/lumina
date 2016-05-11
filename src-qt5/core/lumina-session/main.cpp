//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <QDebug>
#include <QCoreApplication>
#include <QProcess>
#include <QString>

#include "session.h"
#include <LuminaUtils.h>
#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LuminaXDG.h>

#define DEBUG 0

int main(int argc, char ** argv)
{
    if (argc > 1) {
      if (QString(argv[1]) == QString("--version")){
        qDebug() << LUtils::LuminaDesktopVersion();
        return 0;
      }
    }
    if(!QFile::exists(LOS::LuminaShare())){
      qDebug() << "Lumina does not appear to be installed correctly. Cannot find: " << LOS::LuminaShare();
      return 1;
    }
    //Setup any initialization values
    LTHEME::LoadCustomEnvSettings();
    LXDG::setEnvironmentVars();
    setenv("DESKTOP_SESSION","Lumina",1);
    setenv("XDG_CURRENT_DESKTOP","Lumina",1);
    unsetenv("QT_QPA_PLATFORMTHEME"); //causes issues with Lumina themes - not many people have this by default...
    //Check for any missing user config files
    
    //Start X11 if needed

    //Configure X11 monitors if needed

    //Startup the session
    QCoreApplication a(argc, argv);
    LSession sess;
      sess.start();
    int retCode = a.exec();
    qDebug() << "Finished Closing Down Lumina";
    return retCode;
}
