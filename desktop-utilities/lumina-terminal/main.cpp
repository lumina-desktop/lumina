//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <QSystemTrayIcon>
#include <QDebug>

#include <LuminaSingleApplication.h>
#include <LuminaThemes.h>

#include <unistd.h>

#include "TrayIcon.h"
int  main(int argc, char *argv[]) {
   LTHEME::LoadCustomEnvSettings();
   LSingleApplication a(argc, argv, "lumina-terminal");
    if( !a.isPrimaryProcess() ){ return 0; } //poked the current process instead
	
   //First make sure a system tray is available
  /*qDebug() << "Checking for system tray";
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
   }*/
   
   //Now go ahead and setup the app
   LuminaThemeEngine theme(&a);
     QApplication::setQuitOnLastWindowClosed(false);   
     
   //Now start the tray icon
   TrayIcon tray;
    QObject::connect(&a, SIGNAL(InputsAvailable(QStringList)), &tray, SLOT(slotSingleInstance(QStringList)) );
    QObject::connect(&theme, SIGNAL(updateIcons()), &tray, SLOT(updateIcons()) );
    tray.parseInputs(a.inputlist);
   tray.show();
   return  a.exec();
}
