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


#include "LSession.h"
#include "Globals.h"

#include <LuminaXDG.h> //from libLuminaUtils
#include <LuminaThemes.h>
#include <LuminaOS.h>
#include <LuminaUtils.h>

#define DEBUG 0

QFile logfile(QDir::homePath()+"/.lumina/logs/runtime.log");
void MessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg){
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
    if(!QFile::exists(LOS::LuminaShare())){
      qDebug() << "Lumina does not appear to be installed correctly. Cannot find: " << LOS::LuminaShare();
      return 1;
    }
    //Setup any pre-QApplication initialization values
    LXDG::setEnvironmentVars();
    setenv("DESKTOP_SESSION","LUMINA",1);
    setenv("XDG_CURRENT_DESKTOP","LUMINA",1);
    //LSession::setGraphicsSystem("native"); //make sure to use X11 graphics system
    //Setup the log file
    qDebug() << "Lumina Log File:" << logfile.fileName();
    if(QFile::exists(logfile.fileName()+".old")){ QFile::remove(logfile.fileName()+".old"); }
    if(logfile.exists()){ QFile::rename(logfile.fileName(), logfile.fileName()+".old"); }
      //Make sure the parent directory exists
      if(!QFile::exists(QDir::homePath()+"/.lumina/logs")){
        QDir dir;
        dir.mkpath(QDir::homePath()+"/.lumina/logs");
      }
    logfile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTime *timer=0;
    if(DEBUG){ timer = new QTime(); timer->start(); }
    //Startup the Application
    if(DEBUG){ qDebug() << "Session Init:" << timer->elapsed(); }
    LSession a(argc, argv);
    if(DEBUG){ qDebug() << "Theme Init:" << timer->elapsed(); }
    LuminaThemeEngine theme(&a);
    //Setup Log File
    qInstallMessageHandler(MessageOutput);
    if(DEBUG){ qDebug() << "Session Setup:" << timer->elapsed(); }
    a.setupSession();
    if(DEBUG){ qDebug() << "Load Locale:" << timer->elapsed(); }
    LUtils::LoadTranslation(&a, "lumina-desktop");
    //a.LoadLocale(QLocale().name());
    //Start launching external applications
    QTimer::singleShot(2000, &a, SLOT(launchStartupApps()) ); //wait a couple seconds first
    if(DEBUG){ qDebug() << "Exec Time:" << timer->elapsed(); delete timer;}
    int retCode = a.exec();
    //qDebug() << "Stopping the window manager";
    qDebug() << "Finished Closing Down Lumina";
    logfile.close();
    return retCode;
}
