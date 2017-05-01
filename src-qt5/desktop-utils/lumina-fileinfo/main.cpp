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
  //LuminaThemeEngine theme(&a);


  //Read the input variables
  QString path = "";
  QString flag = "";
  for(int i=1; i<argc; i++){
    if( QString(argv[i]).startsWith("-") ){ flag = QString(argv[i]); }
    else{ path = QString(argv[i]); break; }
  }
  //Check the input variables
  // - path
  if(!path.isEmpty()){ path = LUtils::PathToAbsolute(path); }
  // - flag
  if(!flag.isEmpty()){
    if(flag=="-application"){
      flag = "APP"; //for internal use
    }else if(flag=="-link"){
      flag = "LINK"; //for internal use
    }else{
      //Invalid flag - clear the path as well
      flag.clear();
      path.clear();
    }
  }
  if(!path.isEmpty() || !flag.isEmpty()){ 
    MainUI w;
      //QObject::connect(&theme, SIGNAL(updateIcons()), &w, SLOT(UpdateIcons()) );
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
