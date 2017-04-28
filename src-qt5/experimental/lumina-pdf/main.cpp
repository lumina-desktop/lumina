#include <QTranslator>
#include <QApplication>
#include <QDebug>
#include <QFile>

#include "mainUI.h"
#include <LUtils.h>
//#include <LuminaThemes.h>

int main(int argc, char ** argv)
{
  //LTHEME::LoadCustomEnvSettings();
  QApplication a(argc, argv);
  LUtils::LoadTranslation(&a, "l-pdf");
  //LuminaThemeEngine theme(&a);


  //Read the input variables
  QString path = "";
  for(int i=1; i<argc; i++){
    path = LUtils::PathToAbsolute( argv[i] );
    if(QFile::exists(path)){ break; } //already found a valid file 
  }

  MainUI w;
      //QObject::connect(&theme, SIGNAL(updateIcons()), &w, SLOT(UpdateIcons()) );
  if(!path.isEmpty()){  w.loadFile(path); }
  w.show();
  int retCode = a.exec();
  return retCode;
}
