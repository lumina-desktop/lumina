#include <QDebug>
#include <QApplication>
#include <QQuickWidget>

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
    QQuickWidget base;
      qDebug() << "Resize base widget";
    base.resize(1024,768);
      qDebug() << "Load QML File:" << QMLFile;
    base.setSource(QUrl::fromLocalFile(QMLFile));
      qDebug() << "Start Event loop";
    base.show();
    int ret = A.exec();
      qDebug() << " - Finished";
    return ret;
}
