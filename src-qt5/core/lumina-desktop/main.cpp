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
#include <LUtils.h>
#include <LDesktopUtils.h>

#define DEBUG 0

/*QFile logfile;
void MessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg){
  QString txt;
  switch(type){
  case QtDebugMsg:
  	  txt = QString("Debug: %1").arg(msg);
  	  break;
  case QtWarningMsg:
  	  txt = QString("Warning: %1").arg(msg);
	  txt += "\nContext: "+QString(context.file)+" Line: "+QString(context.line)+" Function: "+QString(context.function);
  	  break;
  case QtCriticalMsg:
  	  txt = QString("CRITICAL: %1").arg(msg);
	  txt += "\nContext: "+QString(context.file)+" Line: "+QString(context.line)+" Function: "+QString(context.function);
  	  break;
  case QtFatalMsg:
  	  txt = QString("FATAL: %1").arg(msg);
	  txt += "\nContext: "+QString(context.file)+" Line: "+QString(context.line)+" Function: "+QString(context.function);
  	  break;
  default:
      txt = msg;
  }

  QTextStream out(&logfile);
  out << txt;
  if(!txt.endsWith("\n")){ out << "\n"; }
}*/

int main(int argc, char ** argv)
{
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
    unsetenv("QT_QPA_PLATFORMTHEME"); //causes issues with Lumina themes - not many people have this by default...
    unsetenv("QT_AUTO_SCREEN_SCALE_FACTOR"); //causes pixel-specific scaling issues with the desktop - turn this on after-the-fact for other apps
    //Startup the session
    LSession a(argc, argv);
    if(!a.isPrimaryProcess()){ return 0; }
    //Setup the log file
   /* logfile.setFileName( QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/logs/runtime.log" );
    qDebug() << "Lumina Log File:" << logfile.fileName();
    if(QFile::exists(logfile.fileName()+".old")){ QFile::remove(logfile.fileName()+".old"); }
    if(logfile.exists()){ QFile::rename(logfile.fileName(), logfile.fileName()+".old"); }
      //Make sure the parent directory exists
      if(!QFile::exists(QDir::homePath()+"/.lumina/logs")){
        QDir dir;
        dir.mkpath(QDir::homePath()+"/.lumina/logs");
      }
    logfile.open(QIODevice::WriteOnly | QIODevice::Append);*/
    QTime *timer=0;
    if(DEBUG){ timer = new QTime(); timer->start(); }
    //Setup Log File
    //qInstallMessageHandler(MessageOutput);
    if(DEBUG){ qDebug() << "Theme Init:" << timer->elapsed(); }
    LuminaThemeEngine theme(&a);
    QObject::connect(&theme, SIGNAL(updateIcons()), &a, SLOT(reloadIconTheme()) );
    //if(DEBUG){ qDebug() << "Load Locale:" << timer->elapsed(); }
    //LUtils::LoadTranslation(&a, "lumina-desktop");
    if(DEBUG){ qDebug() << "Session Setup:" << timer->elapsed(); }
    a.setupSession();
    theme.refresh();
    if(DEBUG){ qDebug() << "Exec Time:" << timer->elapsed(); delete timer;}
    int retCode = a.exec();
    //qDebug() << "Stopping the window manager";
    qDebug() << "Finished Closing Down Lumina";
    //logfile.close();
    return retCode;
}
