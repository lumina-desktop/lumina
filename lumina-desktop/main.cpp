//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <QDebug>
//#include <QApplication>
#include <QFile>
#include <QDir>
#include <QString>
#include <QTextStream>
//#include <QDesktopWidget>
//#include <QList>
//#include <QDebug>
#include <QUrl>


//#include "WMProcess.h"
//#include "LDesktop.h"
#include "LSession.h"
#include "Globals.h"

#include <LuminaXDG.h> //from libLuminaUtils

#ifndef PREFIX
#define PREFIX QString("/usr/local")
#endif

QFile logfile(QDir::homePath()+"/.lumina/logs/runtime.log");
void MessageOutput(QtMsgType type, const char *msg){
  QString txt;
  switch(type){
  case QtDebugMsg:
  	  txt = QString("Debug: %1").arg(msg);
  	  break;
  case QtWarningMsg:
  	  txt = QString("Warning: %1").arg(msg);
  	  break;
  case QtCriticalMsg:
  	  txt = QString("CRITICAL: %1").arg(msg);
  	  break;
  case QtFatalMsg:
  	  txt = QString("FATAL: %1").arg(msg);
  	  break;
  }

  QTextStream out(&logfile);
  out << txt;
  if(!txt.endsWith("\n")){ out << "\n"; }
}

int main(int argc, char ** argv)
{
    //Setup any pre-QApplication initialization values
    LXDG::setEnvironmentVars();
    setenv("DESKTOP_SESSION","LUMINA",1);
    setenv("XDG_CURRENT_DESKTOP","LUMINA",1);
    LSession::setGraphicsSystem("native"); //make sure to use X11 graphics system
    //Setup the log file
    qDebug() << "Lumina Log File:" << logfile.fileName();
    if(logfile.exists()){ logfile.remove(); } //remove any old one
      //Make sure the parent directory exists
      if(!QFile::exists(QDir::homePath()+"/.lumina/logs")){
        QDir dir;
        dir.mkpath(QDir::homePath()+"/.lumina/logs");
      }
    logfile.open(QIODevice::WriteOnly | QIODevice::Append);
    //Startup the Application
    LSession a(argc, argv);
    //Setup Log File
    qInstallMsgHandler(MessageOutput);
    a.setupSession();
    //Start up the Window Manager
    //qDebug() << " - Start Window Manager";
    //WMProcess WM;
    //WM.startWM();
    //QObject::connect(&WM, SIGNAL(WMShutdown()), &a, SLOT(exit()) );
    //Load the initial translations
    //QTranslator translator;
    a.LoadLocale(QLocale().name());
    //Now start the desktop
    /*QDesktopWidget DW;
    QList<LDesktop*> screens;
    for(int i=0; i<DW.screenCount(); i++){
      qDebug() << " - Start Desktop " << i;
      screens << new LDesktop(i);
      a.processEvents();
    }*/
    //Start launching external applications
    QTimer::singleShot(1000, &a, SLOT(launchStartupApps()) ); //wait a second first
    //QTimer::singleShot(1000, &a, SLOT(playStartupAudio()) );
    int retCode = a.exec();
    //a.playLogoutAudio();
    //qDebug() << "Stopping the window manager";
    //WM.stopWM();
    qDebug() << "Finished Closing Down Lumina";
    logfile.close();
    return retCode;
}
