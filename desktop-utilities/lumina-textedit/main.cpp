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

#include "PlainTextEditor.h"
#include "syntaxSupport.h"

int  main(int argc, char *argv[]) {
   LTHEME::LoadCustomEnvSettings();
   QApplication a(argc, argv);
   LUtils::LoadTranslation(&a, "lumina-textedit");
   //Now go ahead and setup the app
   LuminaThemeEngine theme(&a);
     
   //Now start the window
   PlainTextEditor W;
    Custom_Syntax *syntax = new Custom_Syntax(W.document());
	syntax->loadRules("C++");
    W.showLineNumbers(true);
    //QObject::connect(&theme, SIGNAL(updateIcons()), &W, SLOT(updateIcons()) );

   W.show();
   return  a.exec();
}
