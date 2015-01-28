//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LuminaUtils.h"

#include <QString>
#include <QFile>
#include <QStringList>
#include <QObject>
#include <QTextCodec>
#include <QDebug>
#include <QDesktopWidget>

#include <LuminaOS.h>
#include <LuminaThemes.h>

//=============
//  LUtils Functions
//=============
QString LUtils::LuminaDesktopVersion(){ 
  return "0.8.2-devel"; 
}

int LUtils::runCmd(QString cmd, QStringList args){
  QProcess *proc = new QProcess;
  proc->setProcessChannelMode(QProcess::MergedChannels);
  if(args.isEmpty()){
    proc->start(cmd);
  }else{
    proc->start(cmd, args);
  }
  while(!proc->waitForFinished(300)){
    QCoreApplication::processEvents();
  }
  int ret = proc->exitCode();
  delete proc;
  return ret;
	
}

QStringList LUtils::getCmdOutput(QString cmd, QStringList args){
  QProcess *proc = new QProcess;
  proc->setProcessChannelMode(QProcess::MergedChannels);
  if(args.isEmpty()){
    proc->start(cmd);
  }else{
    proc->start(cmd,args);	  
  }
  while(!proc->waitForFinished(300)){
    QCoreApplication::processEvents();
  }
  QStringList out = QString(proc->readAllStandardOutput()).split("\n");
  delete proc;
  return out;	
}

QStringList LUtils::readFile(QString filepath){
  QStringList out;
  QFile file(filepath);
  if(file.open(QIODevice::Text | QIODevice::ReadOnly)){
    QTextStream in(&file);
    while(!in.atEnd()){
      out << in.readLine();
    }
    file.close();
  }
  return out;
}

bool LUtils::writeFile(QString filepath, QStringList contents, bool overwrite){
  QFile file(filepath);
  if(file.exists() && !overwrite){ return false; }
  bool ok = false;
  if( file.open(QIODevice::WriteOnly | QIODevice::Truncate) ){
    QTextStream out(&file);
    out << contents.join("\n");
    file.close();
    ok = true;
  }
  return ok;
}

bool LUtils::isValidBinary(QString bin){
  if(!bin.startsWith("/")){
    //Relative path: search for it on the current "PATH" settings
    QStringList paths = QString(qgetenv("PATH")).split(":");
    for(int i=0; i<paths.length(); i++){
      if(QFile::exists(paths[i]+"/"+bin)){ bin = paths[i]+"/"+bin; break;}	    
    }
  }
  //bin should be the full path by now
  if(!bin.startsWith("/")){ return false; }
  QFileInfo info(bin);
  return (info.exists() && info.isExecutable());
}

QStringList LUtils::listSubDirectories(QString dir, bool recursive){
  //This is a recursive method for returning the full paths of all subdirectories (if recursive flag is enabled)
  QDir maindir(dir);
  QStringList out;
  QStringList subs = maindir.entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name);
  for(int i=0; i<subs.length(); i++){
    out << maindir.absoluteFilePath(subs[i]);
    if(recursive){
      out << LUtils::listSubDirectories(maindir.absoluteFilePath(subs[i]), recursive);
    }
  }
  return out;
}

void LUtils::LoadTranslation(QApplication *app, QString appname){
   //Get the current localization
    QString langEnc = "UTF-8"; //default value
    QString langCode = getenv("LANG");
    if(langCode.isEmpty()){ langCode = getenv("LC_ALL"); }
    if(langCode.isEmpty()){ langCode = "en_US.UTF-8"; } //default to US english
    //See if the encoding is included and strip it out as necessary
    if(langCode.contains(".")){
      langEnc = langCode.section(".",-1);
      langCode = langCode.section(".",0,0);
    }
    //Now verify the encoding for the locale
    if(langCode =="C" || langCode=="POSIX" || langCode.isEmpty()){
      langEnc = "System"; //use the Qt system encoding
    }
    if(app !=0){
      qDebug() << "Loading Locale:" << appname << langCode << langEnc;
      //Setup the translator
      QTranslator *CurTranslator = new QTranslator();
      //Use the shortened locale code if specific code does not have a corresponding file
      if(!QFile::exists(LOS::LuminaShare()+"i18n/"+appname+"_" + langCode + ".qm") ){
        langCode.truncate( langCode.indexOf("_") );
      }
      CurTranslator->load( appname+QString("_") + langCode, LOS::LuminaShare()+"i18n/" );
      app->installTranslator( CurTranslator );
    }else{
      //Only going to set the encoding since no application given
      qDebug() << "Loading System Encoding:" << langEnc;
    }
    //Load current encoding for this locale
    QTextCodec::setCodecForLocale( QTextCodec::codecForName(langEnc.toUtf8()) ); 
}

void LUtils::LoadSystemDefaults(bool skipOS){
  //Will create the Lumina configuration files based on the current system template (if any)
  QStringList sysDefaults;
  if(!skipOS){ sysDefaults = LUtils::readFile(LOS::AppPrefix()+"etc/luminaDesktop.conf"); }
  if(sysDefaults.isEmpty() && !skipOS){ sysDefaults = LUtils::readFile(LOS::AppPrefix()+"etc/luminaDesktop.conf.dist"); }
  if(sysDefaults.isEmpty() && !skipOS) { sysDefaults = LUtils::readFile(LOS::SysPrefix()+"etc/luminaDesktop.conf"); }
  if(sysDefaults.isEmpty() && !skipOS){ sysDefaults = LUtils::readFile(LOS::SysPrefix()+"etc/luminaDesktop.conf.dist"); }
  if(sysDefaults.isEmpty() && !skipOS) { sysDefaults = LUtils::readFile("/etc/luminaDesktop.conf"); }
  if(sysDefaults.isEmpty() && !skipOS){ sysDefaults = LUtils::readFile("/etc/luminaDesktop.conf.dist"); }
  if(sysDefaults.isEmpty()){ sysDefaults = LUtils::readFile(LOS::LuminaShare()+"luminaDesktop.conf"); }
  //Find the number of the left-most desktop screen
  QString screen = "0";
  QDesktopWidget *desk =QApplication::desktop();
  for(int i=0; i<desk->screenCount(); i++){
     if(desk->screenGeometry(i).x()==0){ screen = QString::number(i); break; }
  }
  //Now setup the default "desktopsettings.conf" and "sessionsettings.conf" files
  QStringList deskset, sesset;
  //First start with any session settings
  QStringList tmp = sysDefaults.filter("session.");
  sesset << "[General]"; //everything is in this section
  sesset << "DesktopVersion="+LUtils::LuminaDesktopVersion();
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).toLower().simplified();
    QString istrue = (val=="true") ? "true": "false";
    //Now parse the variable and put the value in the proper file   

    if(var=="session.enablenumlock"){ sesset << "EnableNumlock="+ istrue; }
    else if(var=="session.playloginaudio"){ sesset << "PlayStartupAudio="+istrue; }
    else if(var=="session.playlogoutaudio"){ sesset << "PlayLogoutAudio="+istrue; }
  }
  //Now do any desktop settings (only works for the primary desktop at the moment)
  tmp = sysDefaults.filter("desktop.");
  if(!tmp.isEmpty()){deskset << "[desktop-"+screen+"]"; }
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).toLower().simplified();
    //Now parse the variable and put the value in the proper file   
    if(var=="desktop.visiblepanels"){ deskset << "panels="+val; }
    else if(var=="desktop.backgroundfiles"){ deskset << "background\\filelist="+val; }
    else if(var=="desktop.backgroundrotateminutes"){ deskset << "background\\minutesToChange="+val; }
    else if(var=="desktop.plugins"){ deskset << "pluginlist="+val; }
  }
  if(!tmp.isEmpty()){ deskset << ""; } //space between sections
  //Now do any panel1 settings (only works for the primary desktop at the moment)
  tmp = sysDefaults.filter("panel1.");
  if(!tmp.isEmpty()){deskset << "[panel"+screen+".0]"; }
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).toLower().simplified();
    QString istrue = (val=="true") ? "true": "false";
    //Now parse the variable and put the value in the proper file   
    if(var=="panel1.pixelsize"){ deskset << "height="+val; }
    else if(var=="panel1.autohide"){ deskset << "hidepanel="+istrue; }
    else if(var=="panel1.location"){ deskset << "location="+val; }
    else if(var=="panel1.plugins"){ deskset << "pluginlist="+val; }
  }
  if(!tmp.isEmpty()){ deskset << ""; } //space between sections
  //Now do any panel2 settings (only works for the primary desktop at the moment)
  tmp = sysDefaults.filter("panel2.");
  if(!tmp.isEmpty()){deskset << "[panel"+screen+".1]"; }
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).toLower().simplified();
    QString istrue = (val=="true") ? "true": "false";
    //Now parse the variable and put the value in the proper file   
    if(var=="panel2.pixelsize"){ deskset << "height="+val; }
    else if(var=="panel2.autohide"){ deskset << "hidepanel="+istrue; }
    else if(var=="panel2.location"){ deskset << "location="+val; }
    else if(var=="panel2.plugins"){ deskset << "pluginlist="+val; }
  }
  if(!tmp.isEmpty()){ deskset << ""; } //space between sections
  //Now do any menu settings
  tmp = sysDefaults.filter("menu.");
  if(!tmp.isEmpty()){deskset << "[menu]"; }
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).toLower().simplified();
    //Now parse the variable and put the value in the proper file   
    if(var=="menu.plugins"){ deskset << "itemlist="+val; }
  }
  if(!tmp.isEmpty()){ deskset << ""; } //space between sections
  //Now do any theme settings
  QStringList themesettings = LTHEME::currentSettings(); 
      //List: [theme path, colorspath, iconsname, font, fontsize]
  tmp = sysDefaults.filter("theme.");
  bool setTheme = !tmp.isEmpty();
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).toLower().simplified();
    //Now parse the variable and put the value in the proper file   
    if(var=="theme.themefile"){ themesettings[0] = val; }
    else if(var=="theme.colorfile"){ themesettings[1] = val; }
    else if(var=="theme.iconset"){ themesettings[2] = val; }
    else if(var=="theme.font"){ themesettings[3] = val; }
    else if(var=="theme.fontsize"){ themesettings[4] = val; }
  }
  //Ensure that the settings directory exists
  QString setdir = QDir::homePath()+"/.lumina/LuminaDE";
  if(!QFile::exists(setdir)){ 
    QDir dir; 
    dir.mkpath(setdir); 
  }
  //Now save the settings files
  if(setTheme){ LTHEME::setCurrentSettings( themesettings[0], themesettings[1], themesettings[2], themesettings[3], themesettings[4]); }
  LUtils::writeFile(setdir+"/sessionsettings.conf", sesset, true);
  LUtils::writeFile(setdir+"/desktopsettings.conf", deskset, true);
}