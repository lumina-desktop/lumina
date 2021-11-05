#include <QTranslator>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QTextStream>
#include <QDateTime>

#include "../common/SingleApplication.h"
#include "mainUI.h"

int main(int argc, char ** argv)
{
    //Regular performance improvements
    qputenv("QT_NO_GLIB","1");
    //Create/start the application
    SingleApplication a(argc, argv, "trident-networkmgr");
    if(a.isPrimaryProcess()){
      mainUI MW;
      SingleApplication::connect( &a, SIGNAL(InputsAvailable(QStringList)), &MW, SLOT(newInputs(QStringList)) );
      if(MW.size().height() < MW.sizeHint().height()){ MW.adjustSize(); }
      MW.showNormal();
      MW.newInputs(a.inputlist);
      return a.exec();
    }else{
      return 0;
    }
}
