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
#include <QLockFile>
#include <QX11Info>

#include "session.h"
#include <LUtils.h>
#include <LDesktopUtils.h>
#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LuminaXDG.h>

#include <unistd.h>

#define DEBUG 0
int findAvailableSession(){
  int num = 0;
  while(QFile::exists("/tmp/.X11-unix/X"+QString::number(num))){ num++; }
  return num;
}

int main(int argc, char ** argv)
{
    bool unified = false;
    if (argc > 1) {
      if (QString(argv[1]) == QString("--version")){
        qDebug() << LDesktopUtils::LuminaDesktopVersion();
        return 0;
      }else if(QString(argv[1]) == QString("--unified")){
        unified = true;
      }
    }
    if(!QFile::exists(LOS::LuminaShare())){
      qDebug() << "Lumina does not appear to be installed correctly. Cannot find: " << LOS::LuminaShare();
      return 1;
    }
    //Start X11 if needed
    QString disp = QString(getenv("DISPLAY")).simplified();
    if(disp.isEmpty()){
      qDebug() << "No X11 session detected: Lumina will try to start one...";
      //No X session found. Go ahead and re-init this binary within an xinit call
      QString prog = QString(argv[0]).section("/",-1);
      LUtils::isValidBinary(prog); //will adjust the path to be absolute
      if(unified){ prog = prog+" --unified"; }
      QStringList args; args << prog << "--" << ":"+QString::number(findAvailableSession());
      //if(LUtils::isValidBinary("x11vnc")){ args << "--" << "-listen" << "tcp"; } //need to be able to VNC into this session
      return QProcess::execute("xinit", args);
    }
    qDebug() << "Starting the Lumina desktop on current X11 session:" << disp;

    //Check for any stale desktop lock files and clean them up
    QString cfile = QDir::tempPath()+"/.LSingleApp-%1-%2-%3";
    QString desk = "lumina-desktop";
    if(unified){ desk.append("-unified"); }
    cfile = cfile.arg( QString(getlogin()), desk, QString::number(QX11Info::appScreen()) );
    if(QFile::exists(cfile)){
      qDebug() << "Found Desktop Lock for X session:" << disp;
      qDebug() << " - Disabling Lock and starting new desktop session";
      QLockFile lock(cfile+"-lock");
      if(lock.isLocked()){ lock.unlock(); }
      QFile::remove(cfile);
    }
    if(QFile::exists(QDir::tempPath()+"/.luminastopping")){
      QFile::remove(QDir::tempPath()+"/.luminastopping");
    }

    //Configure X11 monitors if needed
    if(LUtils::isValidBinary("lumina-xconfig")){
      qDebug() << " - Resetting monitor configuration to last-used settings";
      QProcess::execute("lumina-xconfig --reset-monitors");
    }
    qDebug() << " - Starting the session...";
    //Setup any initialization values
    LTHEME::LoadCustomEnvSettings();
    LXDG::setEnvironmentVars();
    setenv("DESKTOP_SESSION","Lumina",1);
    setenv("XDG_CURRENT_DESKTOP","Lumina",1);
    unsetenv("QT_QPA_PLATFORMTHEME"); //causes issues with Lumina themes - not many people have this by default...
    //Startup the session
    QApplication a(argc, argv);
      setenv("QT_QPA_PLATFORMTHEME","lthemeengine",1); //make sure this is after the QApplication - not actually using the theme plugin for **this** process
    LSession sess;
      sess.checkFiles(); //Make sure user files are created/installed first
      sess.start(unified);
    int retCode = a.exec();
    qDebug() << "Finished Closing Down Lumina";
    return retCode;
}
