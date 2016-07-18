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
#include <LuminaUtils.h>
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
  //QCoreApplication::exit(0);
}

void LSession::procFinished(){
  //Go through and check the status on all the procs to determine which one finished
  int stopped = 0;
  for(int i=0; i<PROCS.length(); i++){
    if(PROCS[i]->state()==QProcess::NotRunning){
      stopped++;
      if(!stopping){
        //See if this process is the main desktop binary
        if(PROCS[i]->program().section("/",-1) == "lumina-desktop"){ stopall();  } //start closing down everything
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
  proc->start(command, QIODevice::ReadOnly);
  connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(procFinished()) );
  PROCS << proc;
}

void LSession::start(){
  //First check for a valid installation
  if( !LUtils::isValidBinary("fluxbox") || !LUtils::isValidBinary("lumina-desktop") ){
    exit(1);
  }
  //Window Manager First
  // FLUXBOX BUG BYPASS: if the ~/.fluxbox dir does not exist, it will ignore the given config file
  if( !LUtils::isValidBinary("fluxbox") ){
    qDebug() << "[INCOMPLETE LUMINA INSTALLATION] fluxbox binary is missing - cannot continue"; 
  }else{
    QString confDir = QString( getenv("XDG_CONFIG_HOME"))+"/lumina-desktop";
    if(!QFile::exists(confDir)){ QDir dir(confDir); dir.mkpath(confDir); }
    if(!QFile::exists(confDir+"/fluxbox-init")){
      QFile::copy(LOS::LuminaShare()+"/fluxbox-init-rc",confDir+"/fluxbox-init");
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
  if(LUtils::isValidBinary("compton")){ 
    QString set = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/compton.conf";
    if(!QFile::exists(set)){
      QStringList dirs = QString(getenv("XDG_CONFIG_DIRS")).split(":");
      for(int i=0; i<dirs.length(); i++){
        if(QFile::exists(dirs[i]+"/compton.conf")){ QFile::copy(dirs[i]+"/compton.conf", set); break; }
        else if(QFile::exists(dirs[i]+"/compton.conf.sample")){ QFile::copy(dirs[i]+"/compton.conf.sample", set); break; }
      }
    }
    if(!QFile::exists(set)){
      qDebug() << "Using default compton settings";
      startProcess("compositing","compton");
    }else{
      startProcess("compositing","compton --config \""+set+"\"", QStringList() << set);
    }
  }else if(LUtils::isValidBinary("xcompmgr")){ startProcess("compositing","xcompmgr"); }

  //Desktop Next
  startProcess("runtime","lumina-desktop");
  //ScreenSaver
  if(LUtils::isValidBinary("xscreensaver")){ startProcess("screensaver","xscreensaver -no-splash"); }

}
