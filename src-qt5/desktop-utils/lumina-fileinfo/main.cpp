#include <QTranslator>
#include <QApplication>
#include <QDebug>
#include <QFile>

#include "MainUI.h"
#include <LUtils.h>
#include <LuminaThemes.h>

int main(int argc, char ** argv)
{
  LTHEME::LoadCustomEnvSettings();
  QApplication a(argc, argv);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
  LUtils::LoadTranslation(&a, "l-fileinfo");


  //Read the input variables
  QString path = "";
  QString flag = "";
  for(int i=1; i<argc; i++){
    QString tmp(argv[i]);
    if( tmp=="--new-application" ){ flag = "APP"; }
    else if( tmp=="--new-link" ){ flag = "LINK"; }
    else if(!tmp.startsWith("-")){ path = QString(argv[i]); break; }
  }
  //Check the input variables
  // - path
  if(!path.isEmpty()){ path = LUtils::PathToAbsolute(path); }
  if(path.isEmpty() && flag.isEmpty()){ flag = "APP"; }
    MainUI w;
    w.LoadFile(path, flag);
    w.show();
    int retCode = a.exec();
    return retCode;
}
