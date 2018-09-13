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
#include <LDesktopUtils.h>
#include <LuminaSingleApplication.h>

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
  //qDebug() << "Got Process Stopped Signal:";
  for(int i=0; i<PROCS.length(); i++){
    if(PROCS[i]->state()==QProcess::NotRunning){
      //qDebug() << " - Stopped:" << PROCS[i]->objectName();
      stopped++;
      if(!stopping){
        //See if this process is the main desktop binary
        if(PROCS[i]->objectName()=="runtime"){
          qDebug() << "Got Desktop Process Finished:" << PROCS[i]->exitCode();
          //if(PROCS[i]->exitCode()==787){ PROCS[i]->start(QIODevice::ReadOnly); } //special internal restart code
          //else{
          stopall(); //}
        }else if(PROCS[i]->objectName()=="wm" && wmfails<2){ wmfails++; PROCS[i]->start(QIODevice::ReadOnly); wmTimer->start(); } //restart the WM
        //if(PROCS[i]->program().section("/",-1) == "lumina-desktop"){ stopall();  } //start closing down everything
        //else{ PROCS[i]->start(QIODevice::ReadOnly); } //restart the process
        //break;
      }
    }
  }
  //qDebug() << " - Final Count:" << stopped << stopping;
  if(stopping || stopped==PROCS.length()){
    //Note about compton: It does not like running multiple sessions under the *same user*
    // (even on different displays). Run a blanket killall on it when closing down so that
    // any other Lumina sessions will automatically restart compton on that specific display
    QProcess::execute("killall compton");
    QCoreApplication::exit(0);
  }else{
    //Make sure we restart the process as needed
    for(int i=0; i<PROCS.length(); i++){
      if(PROCS[i]->state()==QProcess::NotRunning){
        //runtime/wm processes have special restart rules above
        if(PROCS[i]->objectName()!="runtime" && PROCS[i]->objectName()!="wm"){
          PROCS[i]->start(QIODevice::ReadOnly);
        }
      }
    }
  }
}

void LSession::startProcess(QString ID, QString command, QStringList watchfiles){
  QString dir = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/logs";
  QString display = QString(getenv("DISPLAY")).section(":",1,1);
  if(!QFile::exists(dir)){ QDir tmp(dir); tmp.mkpath(dir); }
  QString logfile = dir+"/"+ID+"_"+display+".log";
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

void LSession::setupCompositor(bool force){
  //Compositing manager
  QSettings settings("lumina-desktop","sessionsettings");
  if(settings.value("enableCompositing",false).toBool() || force){
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
       bool hasAccel = false;
       if(LUtils::isValidBinary("glxinfo")){
	 hasAccel =! LUtils::getCmdOutput("glxinfo -B").filter("direct rendering:").filter("Yes").isEmpty();
	 qDebug() << "Detected GPU Acceleration:" << hasAccel;
	 /*QStringList info = LUtils::readFile(set);
	 for(int i=0; i<info.length(); i++){
	   if(info[i].section("=",0,0).simplified()=="backend"){ info[i] = QString("backend = \"")+ (hasAccel ? "glx" : "xrender")+"\""; break; } //replace this line
	 }
	 LUtils::writeFile(set, info, true);*/
	 if( !hasAccel && settings.value("compositingWithGpuAccelOnly",true).toBool() ){ startcompton = false; }
       }
       QString disp = getenv("DISPLAY");
         // Prefer GLX-accel only if that is detected as possible
	if( (startcompton || force) && hasAccel ){ startProcess("compositing","compton --backend glx -d "+disp+" --config "+set, QStringList() << set); }
         //if cannot determine if GLX accel is available, use the hybrid glx/xrender backend for some better auto-setting stuff
	else if(startcompton || force){ startProcess("compositing","compton --backend xr_glx_hybrid -d "+disp+" --config "+set, QStringList() << set); }
    }else if(LUtils::isValidBinary("xcompmgr") && !settings.value("compositingWithGpuAccelOnly",true).toBool() ){ startProcess("compositing","xcompmgr"); }
  }
}

void LSession::start(bool unified){
  //First check for a valid installation
  if(!LUtils::isValidBinary("lumina-desktop") ){
    exit(1);
  }
  setenv("DESKTOP_SESSION","Lumina",1);
  setenv("XDG_CURRENT_DESKTOP","Lumina",1);
  setenv("QT_QPA_PLATFORMTHEME","lthemeengine", true);
  setenv("QT_NO_GLIB", "1", 1); //Disable the glib event loop within Qt at runtime (performance hit + bugs)
  unsetenv("QT_AUTO_SCREEN_SCALE_FACTOR"); //need exact-pixel measurements (no fake scaling)
  if(LUtils::isValidBinary("xdg-user-dirs-update")){
    //Make sure the XDG user directories are created as needed first
    QProcess::execute("xdg-user-dirs-update");
  }

 if(!unified){
  QSettings sessionsettings("lumina-desktop","sessionsettings");
  QString WM = sessionsettings.value("WindowManager", "fluxbox").toString();
  //Window Manager First
  if(WM=="fluxbox" || WM.endsWith("/fluxbox") || WM.simplified().isEmpty() ){
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
            if(!QFile::exists(confDir+"/fluxbox-overlay")){
              QStringList contents; contents << "background: unset";
              LUtils::writeFile(confDir+"/fluxbox-overlay", contents, true);
              //Now make sure this overlay file is set within the init file
              contents = LUtils::readFile(confDir+"/fluxbox-init");
              contents << "session.styleOverlay:	"+confDir+"/fluxbox-overlay";
              LUtils::writeFile(confDir+"/fluxbox-init", contents, true);
            }
	    // FLUXBOX BUG BYPASS: if the ~/.fluxbox dir does not exist, it will ignore the given config file
	    if(!QFile::exists(QDir::homePath()+"/.fluxbox")){
	      QDir dir; dir.mkpath(QDir::homePath()+"/.fluxbox");
	    }
	    QString cmd = "fluxbox -rc "+confDir+"/fluxbox-init -no-slit -no-toolbar";
	    startProcess("wm", cmd, QStringList() << confDir+"/fluxbox-init" << confDir+"/fluxbox-keys");
	  }
	  //Compositing manager
	  setupCompositor();
  } else {
	if(!LUtils::isValidBinary(WM)){
	  exit(1);
	}
	startProcess("wm", WM);
  }
  //Desktop Next
  LSingleApplication::removeLocks("lumina-desktop");
  startProcess("runtime","lumina-desktop");
  //ScreenSaver
  if(LUtils::isValidBinary("xscreensaver")){ startProcess("screensaver","xscreensaver -no-splash"); }
 }else{
  //unified process
  setupCompositor(true); //required for Lumina 2
  LSingleApplication::removeLocks("lumina-desktop-unified");
  startProcess("runtime","lumina-desktop-unified");
 }
}

void LSession::checkFiles(){
//internal version conversion examples:
  //  [1.0.0 -> 1000000], [1.2.3 -> 1002003], [0.6.1 -> 6001]
  qDebug() << "[Lumina] Checking User Files";
  QSettings sset("lumina-desktop", "sessionsettings");
  QString OVS = sset.value("DesktopVersion","0").toString(); //Old Version String
  qDebug() << " - Old Version:" << OVS;
  qDebug() << " - Current Version:" << LDesktopUtils::LuminaDesktopVersion();
  bool changed = LDesktopUtils::checkUserFiles(OVS, LDesktopUtils::LuminaDesktopVersion());
  qDebug() << " - Made Changes:" << changed;
  if(changed){
    //Save the current version of the session to the settings file (for next time)
    sset.setValue("DesktopVersion", LDesktopUtils::LuminaDesktopVersion());
  }
  qDebug() << "Finished with user files check";
}
