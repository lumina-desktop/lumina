//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LLockScreen.h"
#include "ui_LLockScreen.h"

//Standard C libary for username/system fetch
#include <unistd.h>
//#include <sys/param.h>

//PAM libraries
#include <sys/types.h>
#include <security/pam_appl.h>
#include <security/openpam.h>
//#include <sys/wait.h>
//#include <pwd.h>
//#include <login_cap.h>

#define DEBUG 1

LLockScreen::LLockScreen(QWidget *parent) : QWidget(parent), ui(new Ui::LLockScreen()){
  ui->setupUi(this);
  waittime = new QTimer(this);
    waittime->setInterval(300000); //5 minutes (too many attempts in short time)
    waittime->setSingleShot(true);
  connect(ui->tool_unlock, SIGNAL(clicked()), this, SLOT(TryUnlock()) );
  connect(ui->line_password, SIGNAL(returnPressed()), this, SLOT(TryUnlock()) );
  connect(ui->line_password, SIGNAL(textEdited(QString)), this, SIGNAL(InputDetected()) );
  connect(ui->line_password, SIGNAL(cursorPositionChanged(int,int)), this, SIGNAL(InputDetected()) );
  connect(waittime, SIGNAL(timeout()), this, SLOT(aboutToShow()) );
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
  bool ok = PAM_lockSession(user);
  qDebug() << "Closed PAM session:" << ok;
}

void LLockScreen::aboutToHide(){
  //auto-hide timeout - clear display
  ui->line_password->clear();
  ui->line_password->clearFocus();
  if(!waittime->isActive()){ ui->label_info->clear(); } //could be obsolete the next time the lock screen is shown
  
} 

void LLockScreen::aboutToShow(){
  if(!waittime->isActive()){
    ui->label_info->setText( PAM_checkLockInfo( QString(getlogin()) ) );
    this->setEnabled(true);
    triesleft = 4; //back to initial number of tries
  }
  ui->line_password->clear();
  ui->line_password->setFocus();
} 

// =================
//         PRIVATE
// =================
// PAM structures for the functions below
static struct pam_conv pamc = { openpam_nullconv, NULL };
pam_handle_t *pamh;

bool LLockScreen::PAM_checkpass(QString user, QString pass, QString &info){
  //Convert the inputs to C character arrays for use in PAM
  if(DEBUG){qDebug() << "Check Password w/PAM:" << user << pass;}
  QByteArray tmp2 = pass.toUtf8();
  char* cPassword = tmp2.data();
  //initialize variables
  bool result = false;
    //Place the user-supplied password into the structure 
    int ret = pam_set_item(pamh, PAM_AUTHTOK, cPassword);
    //Set the TTY 
    //Authenticate with PAM
    ret = pam_authenticate(pamh,0);
    if( ret == PAM_SUCCESS ){
      //Check for valid, unexpired account and verify access restrictions
      ret = pam_acct_mgmt(pamh,0);
      if( ret == PAM_SUCCESS ){
        result = true; 
	ret = pam_setcred(pamh,PAM_REINITIALIZE_CRED);
	ret = pam_end(pamh,ret);      
      }else{ info = PAM_getError(ret); }
    }else{
      info = PAM_getError(ret);
    }
  if(DEBUG){ qDebug() << " - Result:" << result << ret; }
  //return verification result
  return result;	
}

QString LLockScreen::PAM_checkLockInfo(QString user){
  //Return info string with any account lock/reset info
  return ""; //not implemented yet
}

QString LLockScreen::PAM_getError(int ret){
  QString err;
  //Interpret a PAM error message and log it
  //qWarning() << "PAM Error: " << ret;
  switch( ret ){
  case PAM_MAXTRIES:
    err = tr("Too Many Failures: Try again later.");
    break;
  case PAM_NEW_AUTHTOK_REQD:
    err = tr("Password Expired: Contact System Admin");
    break;
  default:
    triesleft--;
    if(triesleft>0){ err = QString(tr("Failure: %1 Attempts Remaining")).arg( QString::number(triesleft) ); }
    else{err = tr("Too Many Failures: Try again in 5 minutes"); }
    if(DEBUG){ err.append("\n"+QString(pam_strerror(pamh, ret)) ); }
  }
  return err;
}

bool LLockScreen::PAM_lockSession(QString user){
  QByteArray tmp = user.toUtf8();
  char* cUser = tmp.data();
  int ret = pam_start( user=="root" ? "system": "login", cUser, &pamc, &pamh);
  //if(ret == PAM_SUCCESS){ ret = pam_setcred(pamh,PAM_DELETE_CRED); }
  return (ret== PAM_SUCCESS);
}
// =================
//      PRIVATE SLOTS
// =================
void LLockScreen::TryUnlock(){
  attempts++;
  this->setEnabled(false);
  QString pass = ui->line_password->text();
    ui->line_password->clear();
  QString user = QString(getlogin());
  QString info;
  bool ok = PAM_checkpass(user, pass, info);
  if(ok){
    emit ScreenUnlocked();
    this->setEnabled(true);
  }else if(triesleft>1){
    this->setEnabled(true);
    ui->label_info->setText(info);
  }else{
    ui->label_info->setText(info);
    waittime->start();
  }
}
