#include "BootSplash.h"
#include "ui_BootSplash.h"

#include <LuminaXDG.h>
#include <LUtils.h>
#include <LDesktopUtils.h>

BootSplash::BootSplash() : QWidget(0, Qt::SplashScreen | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus), ui(new Ui::BootSplash){
  ui->setupUi(this);
  this->setObjectName("LuminaBootSplash"); //for theme styling
  //Center the window on the primary screen
  QPoint ctr = QApplication::desktop()->screenGeometry().center();
  this->move( ctr.x()-(this->width()/2), ctr.y()-(this->height()/2) );
  generateTipOfTheDay();
  ui->label_version->setText( QString(tr("Version %1")).arg(LDesktopUtils::LuminaDesktopVersion()) );
}

void BootSplash::generateTipOfTheDay(){
  int index = qrand()%5; //Make sure this number matches the length of the case below (max value +1)
  QString tip = "This desktop is generously sponsored by iXsystems\nwww.ixsystems.com"; //fallback message (just in case)
  switch(index){
    case 0:
	tip = tr("This desktop is powered by coffee, coffee, and more coffee."); break;
    case 1:
	tip = tr("Keep up with desktop news!")+"\n\nwww.lumina-desktop.org"; break;
    case 2:
	tip = tr("There is a full handbook of information about the desktop available online.")+"\n\nwww.lumina-desktop.org/handbook"; break;
    case 3:
	tip = tr("Want to change the interface? Everything is customizable in the desktop configuration!"); break;
    case 4:
	tip = tr("Lumina can easily reproduce the interface from most other desktop environments."); break;
    case 5:
	tip = tr(""); break;
    case 6:
	tip = tr(""); break;
  }
  ui->label_welcome->setText( "\""+tip+"\"" );
}

void BootSplash::showScreen(QString loading){ //update icon, text, and progress
  QString txt, icon;
  int per = 0;
  if(loading=="init"){
    txt = tr("Initializing Session …"); per = 10;
    icon = "preferences-system-login";
  }else if(loading=="settings"){
    txt = tr("Loading System Settings …"); per = 20;	  
    icon = "user-home";
  }else if(loading=="user"){
    txt = tr("Loading User Preferences …"); per = 30; 
    icon = "preferences-desktop-user";
  }else if(loading=="systray"){
    txt = tr("Preparing System Tray …"); per = 40;
    icon = "preferences-plugin";
  }else if(loading=="wm"){
    txt = tr("Starting Window Manager …"); per = 50;
    icon = "preferences-system-windows-actions";	  
  }else if(loading=="apps"){
    txt = tr("Detecting Applications …"); per = 60;
    icon = "preferences-desktop-icons";
  }else if(loading=="menus"){
    txt = tr("Preparing Menus …"); per = 70;
    icon = "preferences-system-windows";
  }else if(loading=="desktop"){
    txt = tr("Preparing Workspace …"); per = 80;
    icon = "preferences-desktop-wallpaper";	
  }else if(loading=="final"){
    txt = tr("Finalizing …"); per = 90;
    icon = "start-here-lumina";	  
  }else if(loading.startsWith("app::")){
    txt = QString(tr("Starting App: %1")).arg(loading.section("::",1,50)); per = -1;
  }
  if(per>0){ ui->progressBar->setValue(per); }
  else{ ui->progressBar->setRange(0,0); } //loading indicator
  ui->label_text->setText(txt);
  if(!icon.isEmpty()){ui->label_icon->setPixmap( LXDG::findIcon(icon, "Lumina-DE").pixmap(64,64) ); }
  this->raise();
  this->show();
  this->update();
  QApplication::processEvents();
}
	
void BootSplash::showText(QString txt){ //will only update the text, not the icon/progress
  ui->label_text->setText(txt);
  this->show();
  this->update();
  QApplication::processEvents();
}
