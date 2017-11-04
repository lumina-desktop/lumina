//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <QApplication>
#include <QDebug>

#include <LuminaThemes.h>
#include <LUtils.h>

#include "MainUI.h"

int  main(int argc, char *argv[]) {
   LTHEME::LoadCustomEnvSettings();
   QApplication a(argc, argv);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
   LUtils::LoadTranslation(&a, "l-archiver");
   //Now go ahead and setup the app
   QStringList args;
   for(int i=1; i<argc; i++){
      if( QString(argv[i]).startsWith("--") ){ args << QString(argv[i]); }
      else{ args << LUtils::PathToAbsolute( QString(argv[i]) ); }
   }
   //Now start the window
   MainUI W;
    W.LoadArguments(args);
   W.show();
   return  a.exec();
}
