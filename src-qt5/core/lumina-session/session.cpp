//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "session.h"

#include <QObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDebug>
#include <QSettings>
#include <QDir>

#include <LUtils.h>
#include <LuminaOS.h>

void LSession::stopall(){
  stopping = true;
  for(int i=0; i<PROCS.length(); i++){
    if(PROCS[i]->state()!=QProcess::NotRunning){ PROCS[i]->kill(); }
  }
  QCoreApplication::processEvents();
  for(int i=0; i<PROCS.length(); i++){
    if(PROCS[i]->state()!=QProcess::NotRunning){ PROCS[i]->terminate(); }
  }
  QCoreApplication::exit(0);
}

void LSession::procFinished(){
  //Go through and check the status on all the procs to determine which one finished
  int stopped = 0;
  for(int i=0; i<PROCS.length(); i++){
    if(PROCS[i]->state()==QProcess::NotRunning){
      stopped++;
      if(!stopping){
        //See if this process is the main desktop binary
        if(PROCS[i]->objectName()=="runtime"){ stopall(); }
        //if(PROCS[i]->program().section("/",-1) == "lumina-desktop"){ stopall();  } //start closing down everything
        //else{ PROCS[i]->start(QIODevice::ReadOnly); } //restart the process
        break;
      }
    }
  }
  if(stopping && stopped==PROCS.length()){
    QCoreApplication::exit(0);
  }
}

void LSession::startProcess(QString ID, QString command, QStringList watchfiles){
  QString dir = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/logs";
  if(!QFile::exists(dir)){ QDir tmp(dir); tmp.mkpath(dir); }
  QString logfile = dir+"/"+ID+".log";
  if(QFile::exists(logfile+".old")){ QFile::remove(logfile+".old"); }
  if(QFile::exists(logfile)){ QFile::rename(logfile,logfile+".old"); }

  LProcess *proc = new LProcess(ID, watchfiles);
  proc->setProcessChannelMode(QProcess::MergedChannels);
  proc->setProcessEnvironment( QProcessEnvironment::systemEnvironment() );
  proc->setStandardOutputFile(logfile);
  proc->setObjectName(ID);
  if(ID=="runtime"){
    //Bypass for a hidden dbus file requirement for Qt itself (Qt 5.5.1+?)
    if(!QFile::exists("/etc/machine-id") && !QFile::exists("/var/db/dbus/machine-id")){
      if(LUtils::isValidBinary("dbus-uuidgen") && LUtils::runCmd("dbus-uuidgen --ensure") ){ } //good - the UUID was created successfully
      else if(LUtils::isValidBinary("dbus-launch")){ command.prepend("dbus-launch --exit-with-session "); }
      else{ 
        //create a simple DBUS UUID and put it in the universal-fallback location (OS-independent)
        // TO-DO - root vs user level permissions issue?
        qDebug() << "Could not find '/etc/machine-id' or '/var/db/dbus/machine-id': Qt will most likely crash. \nPlease run 'dbus-uuidgen --ensure' with root permissions to generate this file if Lumina does not start properly.";
      }
   }
  }
  proc->start(command, QIODevice::ReadOnly);
  connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(procFinished()) );
  PROCS << proc;
}

void LSession::start(bool unified){
  //First check for a valid installation
  if(!LUtils::isValidBinary("lumina-desktop") ){
    exit(1);
  }
 if(!unified){
  QSettings sessionsettings("lumina-desktop","sessionsettings");
  QString WM = sessionsettings.value("WindowManager", "fluxbox").toString();
  //Window Manager First
  if(WM=="fluxbox"){
	  // FLUXBOX BUG BYPASS: if the ~/.fluxbox dir does not exist, it will ignore the given config file
	  if( !LUtils::isValidBinary("fluxbox") ){
	    qDebug() << "[INCOMPLETE LUMINA INSTALLATION] fluxbox binary is missing - cannot continue"; 
	  }else{
	    QString confDir = QString( getenv("XDG_CONFIG_HOME"))+"/lumina-desktop";
	    if(!QFile::exists(confDir)){ QDir dir(confDir); dir.mkpath(confDir); }
	    if(!QFile::exists(confDir+"/fluxbox-init")){
	      QStringList keys = LUtils::readFile(LOS::LuminaShare()+"/fluxbox-init-rc");
	       keys = keys.replaceInStrings("${XDG_CONFIG_HOME}", QString( getenv("XDG_CONFIG_HOME")));
	       LUtils::writeFile(confDir+"/fluxbox-init", keys, true);
	      QFile::setPermissions(confDir+"/fluxbox-init", QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadOther | QFile::ReadGroup);
	    }    
	    if(!QFile::exists(confDir+"/fluxbox-keys")){
	      QStringList keys = LUtils::readFile(LOS::LuminaShare()+"/fluxbox-keys");
	       keys = keys.replaceInStrings("${XDG_CONFIG_HOME}", QString( getenv("XDG_CONFIG_HOME")));
	       LUtils::writeFile(confDir+"/fluxbox-keys", keys, true);
	      QFile::setPermissions(confDir+"/fluxbox-keys", QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadOther | QFile::ReadGroup);
	    }    
	    // FLUXBOX BUG BYPASS: if the ~/.fluxbox dir does not exist, it will ignore the given config file
	    if(!QFile::exists(QDir::homePath()+"/.fluxbox")){
	      QDir dir; dir.mkpath(QDir::homePath()+"/.fluxbox");
	    }
	    QString cmd = "fluxbox -rc "+confDir+"/fluxbox-init -no-slit -no-toolbar";
	    startProcess("wm", cmd, QStringList() << confDir+"/fluxbox-init" << confDir+"/fluxbox-keys");
	  }
	  //Compositing manager
	  QSettings settings("lumina-desktop","sessionsettings");
	  if(settings.value("enableCompositing",false).toBool()){
	    if(LUtils::isValidBinary("compton")){
            //Compton available - check the config file
	      QString set = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/compton.conf";
	      if(!QFile::exists(set)){
		  if(QFile::exists(LOS::LuminaShare()+"/compton.conf")){
		    QFile::copy(LOS::LuminaShare()+"/compton.conf", set);
		  }
	      }
		//Auto-detect if GLX is available on the system and turn it on/off as needed
	       bool startcompton = true;
	       if(LUtils::isValidBinary("glxinfo")){
		 bool hasAccel =! LUtils::getCmdOutput("glxinfo -B").filter("direct rendering:").filter("Yes").isEmpty();
		 qDebug() << "Detected GPU Acceleration:" << hasAccel;
		 QStringList info = LUtils::readFile(set);
		 for(int i=0; i<info.length(); i++){ 
		   if(info[i].section("=",0,0).simplified()=="backend"){ info[i] = QString("backend = \"")+ (hasAccel ? "glx" : "xrender")+"\""; break; } //replace this line
		 }
		 LUtils::writeFile(set, info, true);
		 if( !hasAccel && settings.value("compositingWithGpuAccelOnly",true).toBool() ){ startcompton = false; }
	       }
		if(startcompton && QFile::exists(set)){ startProcess("compositing","compton --config \""+set+"\"", QStringList() << set); }
            else if(startcompton){ startProcess("compositing","compton"); }
	    }else if(LUtils::isValidBinary("xcompmgr") && !settings.value("compositingWithGpuAccelOnly",true).toBool() ){ startProcess("compositing","xcompmgr"); }
	  }
  } else {
	if(!LUtils::isValidBinary(WM)){
	  exit(1);
	}	
	startProcess("wm", WM);
  }
  //Desktop Next
  startProcess("runtime","lumina-desktop");
  //ScreenSaver
  if(LUtils::isValidBinary("xscreensaver")){ startProcess("screensaver","xscreensaver -no-splash"); }
 }else{
  //unified process
  startProcess("runtime","lumina-desktop-unified");
 }
}
