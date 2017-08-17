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
  unsetenv("QT_AUTO_SCREEN_SCALE_FACTOR"); //need pixel-perfect geometries
  QApplication a(argc, argv);
  LUtils::LoadTranslation(&a, "l-pdf");

  //Read the input variables
  QString path = "";
  for(int i=1; i<argc; i++){
    path = LUtils::PathToAbsolute( argv[i] );
    if(QFile::exists(path)){ break; } //already found a valid file
  }

  MainUI w;
  if(!path.isEmpty()){  w.loadFile(path); }
  w.show();
  int retCode = a.exec();
  return retCode;
}
