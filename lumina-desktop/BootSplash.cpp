#include "BootSplash.h"
#include "ui_BootSplash.h"

#include <LuminaXDG.h>

BootSplash::BootSplash() : QWidget(0, Qt::SplashScreen | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus), ui(new Ui::BootSplash){
  ui->setupUi(this);
  this->setObjectName("LuminaBootSplash"); //for theme styling
  //Center the window on the primary screen
  QPoint ctr = QApplication::desktop()->screenGeometry().center();
  this->move( ctr.x()-(this->width()/2), ctr.y()-(this->height()/2) );
}

void BootSplash::showScreen(QString loading){ //update icon, text, and progress
  QString txt, icon;
  int per = 0;
  if(loading=="init"){
    txt = tr("Initializing Session..."); per = 11;
    icon = "preferences-system-login";
  }else if(loading=="settings"){
    txt = tr("Loading Settings..."); per = 22;	  
    icon = "user-home";
  }else if(loading=="user"){
    txt = tr("Checking User Settings..."); per = 33; 
    icon = "preferences-desktop-user";
  }else if(loading=="systray"){
    txt = tr("Registering System Tray..."); per = 44;
    icon = "preferences-plugin";
  }else if(loading=="wm"){
    txt = tr("Starting Window Manager..."); per = 55;
    icon = "preferences-system-windows-actions";	  
  }else if(loading=="desktop"){
    txt = tr("Initializing Desktop(s)..."); per = 66;
    icon = "preferences-desktop-wallpaper";	  
  }else if(loading=="menus"){
    txt = tr("Initializing System Menu(s)..."); per = 77;
    icon = "preferences-system-windows";
  }else if(loading=="final"){
    txt = tr("Performing Final Checks..."); per = 90;
    icon = "pcbsd";	  
  }
  ui->progressBar->setValue(per);
  ui->label_text->setText(txt);
  ui->label_icon->setPixmap( LXDG::findIcon(icon, "Lumina-DE").pixmap(64,64) );
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