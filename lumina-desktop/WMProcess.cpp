//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "WMProcess.h"

WMProcess::WMProcess() : QProcess(){
  connect(this,SIGNAL(finished(int, QProcess::ExitStatus)),this,SLOT(processFinished(int, QProcess::ExitStatus)) );
  this->setProcessChannelMode(QProcess::MergedChannels);
  QString log = QDir::homePath()+"/.lumina/logs/wm.log";
  if(QFile::exists(log)){ QFile::remove(log); }
  this->setStandardOutputFile(log);
  ssaver = new QProcess(0);
  inShutdown = false;
}

WMProcess::~WMProcess(){
	
}

// =======================
//    PUBLIC FUNCTIONS
// =======================
void WMProcess::startWM(){
  inShutdown = false;
  QString cmd = setupWM();
  this->start(cmd);
  ssaver->start("xscreensaver -no-splash");
}

void WMProcess::stopWM(){
  if(isRunning()){
    inShutdown = true;
    //QProcess::startDetached("fluxbox-remote closeallwindows");
    ssaver->kill();
    this->kill();
    if(!this->waitForFinished(10000)){ this->terminate(); };
  }else{
    qWarning() << "WM already closed - did it crash?";
  }
}

void WMProcess::updateWM(){
  if(isRunning()){
    ::kill(this->pid(), SIGUSR2); //send fluxbox the signal to reload it's configuration
  }
}
// =======================
//    PRIVATE FUNCTIONS
// =======================
bool WMProcess::isRunning(){
  return (this->state() != QProcess::NotRunning);
}

QString WMProcess::setupWM(){
  QString WM = "fluxbox";
  QString cmd="echo WM Disabled";
  //leave the option to add other window managers here (for testing purposes)
  if(WM=="openbox"){
    QString confDir = QDir::homePath()+"/.config/openbox";
    if(!QFile::exists(confDir)){ QDir dir(confDir); dir.mkpath(confDir); }
    if(!QFile::exists(confDir+"lumina-rc.xml")){
      QFile::copy(":/openboxconf/lumina-rc.xml",confDir+"/lumina-rc.xml");
      QFile::setPermissions(confDir+"/lumina-rc.xml", QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadOther | QFile::ReadGroup);
    }
    if(!QFile::exists(confDir+"lumina-menu.xml")){
      QFile::copy(":/openboxconf/lumina-menu.xml",confDir+"/lumina-menu.xml");
      QFile::setPermissions(confDir+"/lumina-menu.xml", QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadOther | QFile::ReadGroup);
    }
    //Now copy the configuration files around as necessary
    //if(QFile::exists(confDir+"/rc.xml")){ QFile::rename(confDir+"/rc.xml",confDir+"/openbox-rc.xml"); }
    //QFile::copy(confDir+"/lumina-rc.xml",confDir+"/rc.xml");
    cmd = "openbox --debug --sm-disable --config-file "+confDir+"/lumina-rc.xml";
  }else if(WM=="fluxbox"){
    QString confDir = QDir::homePath()+"/.lumina";
    if(!QFile::exists(confDir)){ QDir dir(confDir); dir.mkpath(confDir); }
    if(!QFile::exists(confDir+"/fluxbox-init")){
      QFile::copy(":/fluxboxconf/fluxbox-init-rc",confDir+"/fluxbox-init");
      QFile::setPermissions(confDir+"/fluxbox-init", QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadOther | QFile::ReadGroup);
    }
    /*if(!QFile::exists(confDir+"lumina-menu.xml")){
      QFile::copy(":/openboxconf/lumina-menu.xml",confDir+"/lumina-menu.xml");
      QFile::setPermissions(confDir+"/lumina-menu.xml", QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadOther | QFile::ReadGroup);
    }*/
    cmd = "fluxbox -rc "+confDir+"/fluxbox-init";
  }
  return cmd;
}

void WMProcess::cleanupConfig(){
  //QString confDir = QDir::homePath()+"/.config/openbox";
  //if(!QFile::exists(confDir+"/rc.xml")){ return; } //Make sure that there is a current config file
  //if(QFile::exists(confDir+"/lumina-rc.xml")){ QFile::remove(confDir+"/lumina-rc.xml"); }
  //QFile::rename(confDir+"/rc.xml",confDir+"/lumina-rc.xml");
  //if(QFile::exists(confDir+"/openbox-rc.xml")){ QFile::rename(confDir+"/openbox-rc.xml",confDir+"/rc.xml"); }
}
// =======================
//     PRIVATE SLOTS
// =======================
void WMProcess::processFinished(int exitcode, QProcess::ExitStatus status){
  if(!inShutdown){
    if(exitcode == 0 && status == QProcess::NormalExit){
      cleanupConfig();
      emit WMShutdown();	  
    }else{
      //restart the Window manager
      this->startWM();
    }
  }else{
    cleanupConfig();	  
  }
}
	
