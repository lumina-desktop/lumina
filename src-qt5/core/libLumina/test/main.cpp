#include <QDebug>
#include <QApplication>

#include "../LuminaRandR.h"


int main(int argc, char** argv){
  QApplication A(argc, argv);
  qDebug() << "Load Monitor Device Information";
  OutputDeviceList devList;
  qDebug() << "Detected Information:";
  for(int i=0; i<devList.length(); i++){
    qDebug() << "["+devList.at(i)->ID()+"]";
    qDebug() << "  - isEnabled:" << devList.at(i)->isEnabled();
    qDebug() << "  - isPrimary:" << devList.at(i)->isPrimary();
    qDebug() << "  - isAutomatic:" << devList.at(i)->isAutomatic();
    qDebug() << "  - Current Geometry:" << devList.at(i)->currentGeometry();
    qDebug() << "  - Physical Size (mm):" << devList.at(i)->physicalSizeMM();
    qDebug() << "  - Available Resolutions:" << devList.at(i)->availableResolutions();
  }
  QString disable = "DVI-I-1";
  qDebug() << "Try Disabling Monitor:" << disable;
  devList.disableMonitor(disable);
  qDebug() << "Finished Test!";
  return 0;
}
