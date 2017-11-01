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
    if( tmp=="-application" ){ flag = "APP"; }
    else if( tmp=="-link" ){ flag = "APP"; }
    else{ path = QString(argv[i]); break; }
  }
  //Check the input variables
  // - path
  if(!path.isEmpty()){ path = LUtils::PathToAbsolute(path); }

  if(!path.isEmpty() || !flag.isEmpty()){
    MainUI w;
    w.LoadFile(path, flag);
    w.show();
    int retCode = a.exec();
    return retCode;
  }else{
    //Show an error text and exit
    qDebug() << "ERROR: Invalid input arguments";
    qDebug() << "Usage: \"lumina-fileinfo [-application | -link] [file]";
    return 1;
  }


}
