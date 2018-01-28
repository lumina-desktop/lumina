//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <QApplication>
#include <QDebug>

#include <LuminaThemes.h>
#include <LUtils.h>

#include "mainUI.h"

int  main(int argc, char *argv[]) {
   LTHEME::LoadCustomEnvSettings();
   QApplication a(argc, argv);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);   
   LUtils::LoadTranslation(&a, "l-calc");
   //Now go ahead and setup the app
   //LuminaThemeEngine theme(&a);
   //Now start the window
   mainUI W;
    //QObject::connect(&theme, SIGNAL(updateIcons()), &W, SLOT(updateIcons()) );
   W.show();
   return  a.exec();
}
