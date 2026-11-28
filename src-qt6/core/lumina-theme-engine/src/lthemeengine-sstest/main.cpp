#include <QApplication>
//#include <QDebug>
#include <QWidget>

#include <LUtils.h>

int main(int argc, char **argv){
  if(argc<2){ return 1; } //error
  unsetenv("QT_QPA_PLATFORMTHEME"); //Make sure we are not testing anything related to the current theme engine
  QString stylesheet = LUtils::readFile(argv[1]).join("\n");
  //qDebug() << "Found Stylesheet:" << stylesheet;
  QApplication app(argc, argv);
  app.setStyleSheet(stylesheet);
  //qDebug() << " Using Stylesheet:" << app.styleSheet();
  QWidget tmp(0,Qt::SplashScreen | Qt::BypassWindowManagerHint);
  tmp.show(); //needed to actually run the parser on the stylesheet (unused/unchecked otherwise)
  return 0;
}
