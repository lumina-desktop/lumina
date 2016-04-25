#include <QTranslator>
#include <QApplication>
#include <QDebug>
#include <QFile>

#include "MainUI.h"
#include <LuminaUtils.h>
#include <LuminaThemes.h>

int main(int argc, char ** argv)
{
  LTHEME::LoadCustomEnvSettings();
  QApplication a(argc, argv);
  LUtils::LoadTranslation(&a, "lumina-fileinfo");
  LuminaThemeEngine theme(&a);


  //Read the input variables
  QString path = "";
  QString flag = "";
  if (argc==2) {
    path = QString::fromLocal8Bit(argv[1]);
  }else if (argc==3) {
    flag = QString::fromLocal8Bit(argv[1]);
    path = QString::fromLocal8Bit(argv[2]);
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
      path.clear();
    }
  }
  if(!path.isEmpty()){ 
    //if(!QFile::exists(path)){ LUtils::writeFile(path,QStringList()); } //create an empty file
    MainUI w;
      QObject::connect(&theme, SIGNAL(updateIcons()), &w, SLOT(UpdateIcons()) );
    w.LoadFile(path, flag);
    w.show();
    int retCode = a.exec();
    return retCode;
  }else{
    //Show an error text and exit
    QStringList msg;
    msg << "ERROR: Invalid input arguments";
    msg << "Usage: \"lumina-fileinfo [-application | -link] <file>";
    qDebug() << msg.join("\n");
    return 1;
  }


}