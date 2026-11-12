#include <QDebug>
#include <QApplication>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QObject>
#include <QStringList>
#include <QTimer>

#include "ListData.h"

int main(int argc, char** argv){
  QString QMLFile;
  for(int i=1; i<argc; i++){
    if(QFile::exists(argv[i])){ QMLFile = QString(argv[i]); }
  }
  if(QMLFile.isEmpty()){
    qDebug() << "No QML File provided!";
    qDebug() << " Please provide a valid qml file path as an input argument";
    return 1;
  }
  QApplication A(argc,argv);
      qDebug() << "Creating base widget";
    ListData data;
    QQuickView base;
    base.setResizeMode(QQuickView::SizeRootObjectToView);
      qDebug() << "Resize base widget";
    base.resize(1024,768);
   base.engine()->rootContext()->setContextProperty("cppdata", &data);
      qDebug() << "Load QML File:" << QMLFile;
    base.setSource(QUrl::fromLocalFile(QMLFile));
      qDebug() << "Start Event loop";
    base.show();
    int ret = A.exec();
      qDebug() << " - Finished";
    return ret;
}
