#include "SystemWindow.h"
#include "ui_SystemWindow.h"

#include "LSession.h"
#include <LuminaOS.h>
#include <QPoint>
#include <QCursor>
#include <QDebug>
#include <QProcess>
#include <QDesktopWidget>

SystemWindow::SystemWindow() : QDialog(), ui(new Ui::SystemWindow){
  ui->setupUi(this); //load the designer file
  //Setup the window flags
  this->setWindowFlags( Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  //Setup the icons based on the current theme
  ui->tool_logout->setIcon( LXDG::findIcon("system-log-out","") );
  ui->tool_restart->setIcon( LXDG::findIcon("system-reboot","") );
  ui->tool_shutdown->setIcon( LXDG::findIcon("system-shutdown","") );
  ui->tool_suspend->setIcon( LXDG::findIcon("system-suspend","") );
  ui->push_cancel->setIcon( LXDG::findIcon("dialog-cancel","") );
  ui->push_lock->setIcon( LXDG::findIcon("system-lock-screen","") );
  //Connect the signals/slots
  connect(ui->tool_logout, SIGNAL(clicked()), this, SLOT(sysLogout()) );
  connect(ui->tool_restart, SIGNAL(clicked()), this, SLOT(sysRestart()) );
  connect(ui->tool_shutdown, SIGNAL(clicked()), this, SLOT(sysShutdown()) );
  connect(ui->tool_suspend, SIGNAL(clicked()), this, SLOT(sysSuspend()) );
  connect(ui->push_cancel, SIGNAL(clicked()), this, SLOT(sysCancel()) );
  connect(ui->push_lock, SIGNAL(clicked()), this, SLOT(sysLock()) );
  //Disable the shutdown/restart buttons if necessary
  if( !LOS::userHasShutdownAccess() ){
    ui->tool_restart->setEnabled(false);
    ui->tool_shutdown->setEnabled(false);
   
  }
  ui->tool_suspend->setVisible(LOS::systemCanSuspend());
  //Center this window on the screen
  QPoint center = QApplication::desktop()->screenGeometry(QCursor::pos()).center(); //get the center of the current screen
  this->move(center.x() - this->width()/2, center.y() - this->height()/2);
  this->show();
}

SystemWindow::~SystemWindow(){

}

void SystemWindow::sysLogout(){
  QTimer::singleShot(0, LSession::handle(), SLOT(StartLogout()) );
  this->close();
}
	
void SystemWindow::sysRestart(){
  QTimer::singleShot(0, LSession::handle(), SLOT(StartReboot()) );
  this->close();	
}
	
void SystemWindow::sysShutdown(){
  QTimer::singleShot(0, LSession::handle(), SLOT(StartShutdown()) );
  this->close();
}

void SystemWindow::sysSuspend(){
  //Make sure to lock the system first (otherwise anybody can access it again)
  LUtils::runCmd("xscreensaver-command -lock");
  //Now suspend the system
  LOS::systemSuspend();
  this->close();
}

void SystemWindow::sysLock(){
  qDebug() << "Locking the desktop...";
  QProcess::startDetached("xscreensaver-command -lock");
  this->close();
}
