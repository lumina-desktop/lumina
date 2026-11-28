#include <QDebug>
#include <QApplication>

#include "../LuminaRandR.h"


int main(int argc, char** argv){
  QString toggle = "DVI-I-1";
  QRect toggleGeom(7680,0, 1024,768);
  int toggleOK = -1; //-1: automatic, 0: enable monitor, 1: disable monitor

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
      if(devList.at(i)->ID() == toggle && toggleOK<0){ toggleOK = (devList.at(i)->isEnabled() ? 1 : 0); }
    }
  }
  qDebug() << "\n================\n";
  if(toggleOK == 0){
    qDebug() << "Try Enabling Monitor:" << toggle << toggleGeom;
    bool ok = devList.enableMonitor(toggle, toggleGeom);
    qDebug() << " -- Success:" << ok;
  }else if(toggleOK == 1){
    qDebug() << "Try Disabling Monitor:" << toggle;
    bool ok = devList.disableMonitor(toggle);
    qDebug() << " -- Success:" << ok;
  }

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
