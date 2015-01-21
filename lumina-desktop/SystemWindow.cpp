#include "SystemWindow.h"
#include "ui_SystemWindow.h"

#include "LSession.h"
#include <LuminaOS.h>
#include <unistd.h> //for usleep() usage
#include <QPoint>
#include <QCursor>

SystemWindow::SystemWindow() : QDialog(), ui(new Ui::SystemWindow){
  ui->setupUi(this); //load the designer file
  //Setup the window flags
  this->setWindowFlags( Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  //Setup the icons based on the current theme
  ui->tool_logout->setIcon( LXDG::findIcon("system-log-out","") );
  ui->tool_restart->setIcon( LXDG::findIcon("system-reboot","") );
  ui->tool_shutdown->setIcon( LXDG::findIcon("system-shutdown","") );
  ui->push_cancel->setIcon( LXDG::findIcon("dialog-cancel","") );
  ui->push_lock->setIcon( LXDG::findIcon("system-lock-screen","") );
  //Connect the signals/slots
  connect(ui->tool_logout, SIGNAL(clicked()), this, SLOT(sysLogout()) );
  connect(ui->tool_restart, SIGNAL(clicked()), this, SLOT(sysRestart()) );
  connect(ui->tool_shutdown, SIGNAL(clicked()), this, SLOT(sysShutdown()) );
  connect(ui->push_cancel, SIGNAL(clicked()), this, SLOT(sysCancel()) );
  connect(ui->push_lock, SIGNAL(clicked()), this, SLOT(sysLock()) );
  //Disable the shutdown/restart buttons if necessary
  if( !LOS::userHasShutdownAccess() ){
    ui->tool_restart->setEnabled(false);
    ui->tool_shutdown->setEnabled(false);
  }
  //Center this window on the screen
  QPoint center = QApplication::desktop()->screenGeometry(QCursor::pos()).center(); //get the center of the current screen
  this->move(center.x() - this->width()/2, center.y() - this->height()/2);
  this->show();
}

SystemWindow::~SystemWindow(){

}

void SystemWindow::closeAllWindows(){
  this->hide();
  LSession::processEvents();
  if( LSession::handle()->sessionSettings()->value("PlayLogoutAudio",true).toBool() ){
    LSession::handle()->playAudioFile(LOS::LuminaShare()+"Logout.ogg");
  }
  QList<WId> WL = LX11::WindowList();
  for(int i=0; i<WL.length(); i++){
    LX11::CloseWindow(WL[i]);
    LSession::processEvents();
  }
  //Now go through the list again and kill any remaining windows
  usleep(60); //60 ms pause
  WL = LX11::WindowList();
  for(int i=0; i<WL.length(); i++){
    LX11::KillWindow(WL[i]);
    LSession::processEvents();
  }
  LSession::processEvents();
}
