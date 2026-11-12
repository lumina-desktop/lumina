#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include "lthemeengine.h"
#include <QTranslator>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QStyleFactory>
#include "mainwindow.h"

int main(int argc, char **argv){
  QApplication app(argc, argv);
  QTranslator translator;
  QString locale = lthemeengine::systemLanguageID();
  translator.load(QString(":/lthemeengine_") + locale);
  app.installTranslator(&translator);
  QTranslator qt_translator;
  qt_translator.load(QLibraryInfo::location (QLibraryInfo::TranslationsPath) + "/qtbase_" + locale);
  app.installTranslator(&qt_translator);
  qDebug("Configuration path: %s", qPrintable(lthemeengine::configPath()));
  qDebug("Shared QSS path: %s", qPrintable(lthemeengine::sharedStyleSheetPath().join(", ")));
  //checking environment
  QStringList errorMessages;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  if(env.contains("QT_STYLE_OVERRIDE")){ errorMessages << app.translate("main", "Please remove the <b>QT_STYLE_OVERRIDE</b> environment variable"); }
  //if(env.value("QT_QPA_PLATFORMTHEME") != "lthemeengine"){ errorMessages << app.translate("main", "The <b>QT_QPA_PLATFORMTHEME</b> environment variable is not set correctly"); }
  if(!QStyleFactory::keys().contains("lthemeengine-style")){ errorMessages << app.translate("main", "Unable to find <b>liblthemeengine-style.so</b>"); }
  if(!errorMessages.isEmpty()){
    QMessageBox::critical(0, app.translate("main", "Error"), errorMessages.join("<br><br>"));
    return 0;
    }
  MainWindow w;
  w.show();
  return app.exec();
}
