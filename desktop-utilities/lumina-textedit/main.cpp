//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <QApplication>
#include <QDebug>

#include <LuminaThemes.h>
#include <LuminaUtils.h>

#include "MainUI.h"

int  main(int argc, char *argv[]) {
   LTHEME::LoadCustomEnvSettings();
   QApplication a(argc, argv);
   LUtils::LoadTranslation(&a, "lumina-textedit");
   //Now go ahead and setup the app
   LuminaThemeEngine theme(&a);
   QStringList args;
   for(int i=1; i<argc; i++){
      args << QString(argv[i]);
   }
   //Now start the window
   MainUI W;
    QObject::connect(&theme, SIGNAL(updateIcons()), &W, SLOT(updateIcons()) );
    W.LoadArguments(args);
   W.show();
   return  a.exec();
}
