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
    //Start X11 if needed
    QString disp = QString(getenv("DISPLAY")).simplified();
    if(disp.isEmpty()){
      //No X session found. Go ahead and re-init this binary within an xinit call
      QString prog = QCoreApplication::applicationFilePath().section("/",-1);
      LUtils::isValidBinary(prog); //will adjust the path to be absolute
      QStringList args; args << prog;
      //if(LUtils::isValidBinary("x11vnc")){ args << "--" << "-listen" << "tcp"; } //need to be able to VNC into this session
      return QProcess::execute("xinit", args);
    }
    //Setup any initialization values
    LTHEME::LoadCustomEnvSettings();
    LXDG::setEnvironmentVars();
    setenv("DESKTOP_SESSION","Lumina",1);
    setenv("XDG_CURRENT_DESKTOP","Lumina",1);
    unsetenv("QT_QPA_PLATFORMTHEME"); //causes issues with Lumina themes - not many people have this by default...
    //Check for any missing user config files
    


    //Configure X11 monitors if needed
    if(LUtils::isValidBinary("lumina-xconfig")){ 
      QProcess::execute("lumina-xconfig --reset-monitors");
    }
    //Startup the session
    QCoreApplication a(argc, argv);
    LSession sess;
      sess.start();
    int retCode = a.exec();
    qDebug() << "Finished Closing Down Lumina";
    return retCode;
}
