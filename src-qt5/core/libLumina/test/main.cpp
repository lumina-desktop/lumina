
#include "../LuminaRandR.h"
#include <QDebug>
#include <QApplication>

int main(int argc, char** argv){
  QApplication A(argc, argv); 
  qDebug() << "Starting monitor scan...";
  QList<OutputDevice> outputs = OutputDevice::availableMonitors();
  qDebug() << "Finished monitor Scan";
  OutputDeviceList();
  return 0;
}
