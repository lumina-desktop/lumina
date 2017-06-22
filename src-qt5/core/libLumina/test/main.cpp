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
    qDebug() << "  - isConnected:" << devList.at(i)->isConnected();
    if(devList.at(i)->isConnected()){
      qDebug() << "  - isEnabled:" << devList.at(i)->isEnabled();
      qDebug() << "  - isPrimary:" << devList.at(i)->isPrimary();
      qDebug() << "  - Current Geometry:" << devList.at(i)->currentGeometry();
      qDebug() << "  - Physical Size (mm):" << devList.at(i)->physicalSizeMM();
      qDebug() << "  - Current DPI:" << devList.at(i)->physicalDPI();
      qDebug() << "  - Available Resolutions:" << devList.at(i)->availableResolutions();
    }
  }
  /*QString disable = "HDMI-2";
  qDebug() << "Try Disabling Monitor:" << disable;
  devList.disableMonitor(disable);*/

  /*QString setprimary = "eDP-1";
  if(devList.primaryMonitor() != setprimary){
    qDebug() << "Try setting monitor as primary:" << setprimary;
    devList.setPrimaryMonitor(setprimary);
    //Now see if the status changed on X itself
    for(int i=0; i<devList.length(); i++){
      if(devList.at(i)->ID() == setprimary){
        devList.at(i)->updateInfoCache(); //resync with X server info
        qDebug() << "Successful:" << devList.at(i)->isPrimary();
      }
    }
  }*/

  qDebug() << "[  Finished Test!  ]";
  return 0;
}
