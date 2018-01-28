//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LLockScreen.h"
#include "ui_LLockScreen.h"

#include <unistd.h>

#define NUMTRIES 3
#define DEBUG 1

LLockScreen::LLockScreen(QWidget *parent) : QWidget(parent), ui(new Ui::LLockScreen()){
  ui->setupUi(this);
  waittime = new QTimer(this);
    waittime->setSingleShot(true);
  refreshtime = new QTimer(this); //timer to update the wait time display
    refreshtime->setInterval(6000); //6 seconds (1/10 second)

  connect(ui->tool_unlock, SIGNAL(clicked()), this, SLOT(TryUnlock()) );
  connect(ui->line_password, SIGNAL(returnPressed()), this, SLOT(TryUnlock()) );
  connect(ui->line_password, SIGNAL(textEdited(QString)), this, SIGNAL(InputDetected()) );
  connect(ui->line_password, SIGNAL(cursorPositionChanged(int,int)), this, SIGNAL(InputDetected()) );
  connect(waittime, SIGNAL(timeout()), this, SLOT(aboutToShow()) );
  connect(refreshtime, SIGNAL(timeout()), this, SLOT(UpdateLockInfo()) );
}

LLockScreen::~LLockScreen(){

}

void LLockScreen::LoadSystemDetails(){
   //Run every time the screen is initially locked
   QString user = QString(getlogin());
   ui->label_username->setText( QString(tr("Locked by: %1")).arg(user) );
   ui->label_hostname->setText( QHostInfo::localHostName() );
   ui->tool_unlock->setIcon( LXDG::findIcon("document-decrypt","") );
   attempts = 0;
}

void LLockScreen::aboutToHide(){
  //auto-hide timeout - clear display
  ui->line_password->clear();
  ui->line_password->clearFocus();
  if(refreshtime->isActive()){ refreshtime->stop(); }
}

void LLockScreen::aboutToShow(){
  if(!waittime->isActive()){
    ui->label_info->clear();
    this->setEnabled(true);
    triesleft = NUMTRIES; //back to initial number of tries
    if(refreshtime->isActive()){ refreshtime->stop(); }
  }else{
    if(!refreshtime->isActive()){ refreshtime->start(); }
  }
  UpdateLockInfo();
  ui->line_password->clear();
  ui->line_password->setFocus();
}

// =================
//      PRIVATE SLOTS
// =================
void LLockScreen::UpdateLockInfo(){
  QString info;
  if(waittime->isActive()){
    info = tr("Too Many Failures")+"\n"+ QString(tr("Wait %1 Minutes")).arg( QString::number(qRound(waittime->remainingTime()/6000.0)/10.0) );
  }else if(attempts>0){ info.append("\n"+QString(tr("Failed Attempts: %1")).arg(QString::number(attempts)) ); }
  ui->label_info->setText(info);
}

void LLockScreen::TryUnlock(){
  attempts++;
  this->setEnabled(false);
  QString pass = ui->line_password->text();
    ui->line_password->clear();
  //Create a temporary file for the password, then pass that file descriptor to lumina-checkpass
  QTemporaryFile *TF = new QTemporaryFile(".XXXXXXXXXX");
  TF->setAutoRemove(true);
  bool ok = false;
  if( TF->open() ){
    QTextStream in(TF);
    in << pass.toUtf8()+"\0"; //make sure it is null-terminated
    TF->close();
    //qDebug() << "Trying to unlock session:" << TF->fileName() << LUtils::readFile(TF->fileName());
    //qDebug() << "UserName:" << getlogin();
    LUtils::runCommand(ok, "lumina-checkpass",QStringList() << "-f" << TF->fileName() );
  }
  delete TF; //ensure the temporary file is removed **right now** for security purposes
  if(ok){
    emit ScreenUnlocked();
    this->setEnabled(true);
  }else{
    triesleft--;
    if(triesleft>0){
      this->setEnabled(true);
    }else{
      waittime->setInterval( (attempts/NUMTRIES)*60000);
      waittime->start();
      refreshtime->start();
    }
    ui->line_password->setFocus();
  }
  UpdateLockInfo();

}
