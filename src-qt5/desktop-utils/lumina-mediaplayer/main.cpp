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
#include <LuminaSingleApplication.h>

#include "mainUI.h"

int  main(int argc, char *argv[]) {
   LTHEME::LoadCustomEnvSettings();
   LSingleApplication a(argc, argv, "l-mediap");
   //Now go ahead and setup the app
   QStringList args;
   for(int i=1; i<argc; i++){
      if( QString(argv[i]).startsWith("--") ){ args << QString(argv[i]); }
      else{ args << LUtils::PathToAbsolute( QString(argv[i]) ); }
   }
   //Now start the window
   MainUI W;
    W.loadArguments(args);
   W.show();
   return  a.exec();
}
