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
  //Disable buttons if necessary
  updateWindow();
  ui->tool_suspend->setVisible(LOS::systemCanSuspend()); //does not change with time - just do a single check
  connect(QApplication::instance(), SIGNAL(LocaleChanged()), this, SLOT(updateWindow()) );
}

SystemWindow::~SystemWindow(){

}

void SystemWindow::updateWindow(){
  //Disable the shutdown/restart buttons if necessary
  ui->retranslateUi(this);
  bool ok = LOS::userHasShutdownAccess();
    ui->tool_restart->setEnabled(ok);
    ui->tool_shutdown->setEnabled(ok);
  //Center this window on the current screen
  QPoint center = QApplication::desktop()->screenGeometry(QCursor::pos()).center(); //get the center of the current screen
  this->move(center.x() - this->width()/2, center.y() - this->height()/2);
}

void SystemWindow::sysLogout(){
  this->hide();
  LSession::processEvents();
  QTimer::singleShot(0, LSession::handle(), SLOT(StartLogout()) );
}
	
void SystemWindow::sysRestart(){
  this->hide();
  LSession::processEvents();
  QTimer::singleShot(0, LSession::handle(), SLOT(StartReboot()) );
}
	
void SystemWindow::sysShutdown(){
  this->hide();
  LSession::processEvents();
  QTimer::singleShot(0, LSession::handle(), SLOT(StartShutdown()) );
}

void SystemWindow::sysSuspend(){
  this->hide();
  LSession::processEvents();
  //Make sure to lock the system first (otherwise anybody can access it again)
  LUtils::runCmd("xscreensaver-command -lock");
  //Now suspend the system
  LOS::systemSuspend();
}

void SystemWindow::sysLock(){
  this->hide();
  LSession::processEvents();
  qDebug() << "Locking the desktop...";
  QProcess::startDetached("xscreensaver-command -lock");
}
