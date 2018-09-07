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


#include "LPanel.h"
#include "Globals.h"

#include <LuminaXDG.h> //from libLuminaUtils
#include <LuminaOS.h>
#include <LUtils.h>
#include <LDesktopUtils.h>
#include <AppMenu.h>
#include <LSession.h>

#define DEBUG 0

int main(int argc, char ** argv)
{
    QString monitorID;
    int panelnum = 0;
    if (argc > 1) {
      monitorID = QString(argv[1]);
    }

    //Setup any pre-QApplication initialization values
    LXDG::setEnvironmentVars();

    setenv("QT_QPA_PLATFORMTHEME", "lthemeengine", 1);
    unsetenv("QT_AUTO_SCREEN_SCALE_FACTOR"); //causes pixel-specific scaling issues with the desktop - turn this on after-the-fact for other apps
    //Startup the session
    LSession a(argc, argv);
    a.setupSession(true);
    if(monitorID.isEmpty()){
      monitorID = a.screens().first()->name();
    }
    //qDebug() << "Create AppMenu";
    //AppMenu App;
    //qDebug() << "Create Panel" << a.sessionSettings()->fileName();
    QSettings settings("lumina-desktop","desktopsettings");
    LPanel P(&settings, monitorID, panelnum, 0 , false);
    P.show();
    int retCode = a.exec();
    return retCode;
}
