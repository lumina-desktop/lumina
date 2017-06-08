
#include "../LuminaRandR.h"
#include <QDebug>
#include <QApplication>

int main(int argc, char** argv){
  QApplication A(argc, argv); 
  qDebug() << "Load Monitor Device Information";
  OutputDeviceList devList;
  qDebug() << "Detected Information:";
  for(int i=0; i<devList.length(); i++){
    qDebug() << "["+devList[i].ID()+"]";
    qDebug() << "  - isEnabled:" << devList[i].isEnabled();
    qDebug() << "  - isPrimary:" << devList[i].isPrimary();
    qDebug() << "  - isAutomatic:" << devList[i].isAutomatic();
    qDebug() << "  - Current Geometry:" << devList[i].currentGeometry();
    qDebug() << "  - Available Resolutions:" << devList[i].availableResolutions();
  }
  return 0;
}
