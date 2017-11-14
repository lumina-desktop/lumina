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
    //unsetenv("QT_QPA_PLATFORMTHEME"); //causes issues with Lumina themes - not many people have this by default...
    setenv("QT_QPA_PLATFORMTHEME", "lthemeengine", 1);
    unsetenv("QT_AUTO_SCREEN_SCALE_FACTOR"); //causes pixel-specific scaling issues with the desktop - turn this on after-the-fact for other apps
    //Startup the session
    LSession a(argc, argv);
    if(!a.isPrimaryProcess()){ return 0; }
    //Ensure that the user's config files exist
    /*if( LSession::checkUserFiles() ){  //make sure to create any config files before creating the QApplication
      qDebug() << "User files changed - restarting the desktop session";
      return 787; //return special restart code
    }*/
    //Setup the log file
    QTime *timer=0;
    if(DEBUG){ timer = new QTime(); timer->start(); }
    if(DEBUG){ qDebug() << "Session Setup:" << timer->elapsed(); }
    a.setupSession();
    if(DEBUG){ qDebug() << "Exec Time:" << timer->elapsed(); delete timer;}
    int retCode = a.exec();
    //qDebug() << "Stopping the window manager";
    qDebug() << "Finished Closing Down Lumina";
    return retCode;
}
