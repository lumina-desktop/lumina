#include <QApplication>
#include <QSystemTrayIcon>
#include "Trayapp.h"

#include <unistd.h>

int  main(int argc, char *argv[]) {

   QApplication a(argc, argv);

   bool ready = false;
   for(int i=0; i<60 && !ready; i++){
      ready = QSystemTrayIcon::isSystemTrayAvailable();
      if(!ready){
	//Pause for 5 seconds
        sleep(5); //don't worry about stopping event handling - nothing running yet
      }
   }
   if(!ready){
     qDebug() << "Could not find any available system tray after 5 minutes: exiting....";
     return 1;
   }
   
   TrayApp tray;
   tray.setStyleSheet(background-color: #999999);
   tray.show();
   QApplication::setQuitOnLastWindowClosed(false); 
   return  a.exec();
}
