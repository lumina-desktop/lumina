//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013-2015, Ken Moore
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
#include <QImageReader>

#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LuminaXDG.h>

static QStringList fav;
//=============
//  LUtils Functions
//=============
QString LUtils::LuminaDesktopVersion(){ 
  return "0.8.5-devel"; 
}

int LUtils::runCmd(QString cmd, QStringList args){
  QProcess proc;
  proc.setProcessChannelMode(QProcess::MergedChannels);
  if(args.isEmpty()){
    proc.start(cmd);
  }else{
    proc.start(cmd, args);
  }
  while(!proc.waitForFinished(300)){
    QCoreApplication::processEvents();
  }
  int ret = proc.exitCode();
  return ret;
	
}

QStringList LUtils::getCmdOutput(QString cmd, QStringList args){
  QProcess proc;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  proc.setProcessEnvironment(env);
  proc.setProcessChannelMode(QProcess::MergedChannels);
  if(args.isEmpty()){
    proc.start(cmd);
  }else{
    proc.start(cmd,args);	  
  }
  while(!proc.waitForFinished(500)){
    QCoreApplication::processEvents();
  }
  QStringList out = QString(proc.readAllStandardOutput()).split("\n");
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
  if(contents.isEmpty()){ contents << "\n"; }
  if( file.open(QIODevice::WriteOnly | QIODevice::Truncate) ){
    QTextStream out(&file);
    out << contents.join("\n");
    if(!contents.last().isEmpty()){ out << "\n"; } //always end with a new line
    file.close();
    ok = true;
  }
  return ok;
}

bool LUtils::isValidBinary(QString& bin){
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
  bool good = (info.exists() && info.isExecutable());
  if(good){ bin = info.absoluteFilePath(); }
  return good;
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

QStringList LUtils::imageExtensions(){
  //Note that all the image extensions are lowercase!!
  static QStringList imgExtensions;
  if(imgExtensions.isEmpty()){
    QList<QByteArray> fmt = QImageReader::supportedImageFormats();
    for(int i=0; i<fmt.length(); i++){ imgExtensions << QString::fromLocal8Bit(fmt[i]); }
  }
  return imgExtensions;
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

double LUtils::DisplaySizeToBytes(QString num){
  //qDebug() << "Convert Num to Bytes:" << num;
  num = num.toLower().simplified();
  num = num.remove(" ");
  if(num.isEmpty()){ return 0.0; }
  if(num.endsWith("b")){ num.chop(1); } //remove the "bytes" marker (if there is one)
  QString lab = "b";
  if(!num[num.size()-1].isNumber()){
    lab = num.right(1); num.chop(1);
  }
  double N = num.toDouble();
  QStringList labs; labs <<"b"<<"k"<<"m"<<"g"<<"t"<<"p"; //go up to petabytes for now
  for(int i=0; i<labs.length(); i++){
    if(lab==labs[i]){ break; }//already at the right units - break out
    N = N*1024.0; //Move to the next unit of measurement
  }
  //qDebug() << " - Done:" << QString::number(N) << lab << num;
  return N;
}

//Various function for finding valid QtQuick plugins on the system
bool LUtils::validQuickPlugin(QString ID){
  return ( !LUtils::findQuickPluginFile(ID).isEmpty() );
}

QString LUtils::findQuickPluginFile(QString ID){
  if(ID.startsWith("quick-")){ ID = ID.section("-",1,50); }
  //Give preference to any user-supplied plugins (overwrites for system plugins)
  QString path = QDir::homePath()+"/.lumina/quickplugins/"+ID+".qml";
  if( QFile::exists(path) ){return path; }
  path = LOS::LuminaShare()+"quickplugins/"+ID+".qml";
  if( QFile::exists(path) ){return path; }
  return ""; //could not be found
}

QStringList LUtils::listFavorites(){
  static QDateTime lastRead;
  QDateTime cur = QDateTime::currentDateTime();
  if(lastRead.isNull() || lastRead<QFileInfo(QDir::homePath()+"/.lumina/favorites/fav.list").lastModified()){
    fav = LUtils::readFile(QDir::homePath()+"/.lumina/favorites/fav.list");
    fav.removeAll(""); //remove any empty lines
    lastRead = cur;
    if(fav.isEmpty()){
      //Make sure the favorites dir exists, and create it if necessary
      QDir dir(QDir::homePath()+"/.lumina/favorites");
	if(!dir.exists()){ dir.mkpath(QDir::homePath()+"/.lumina/favorites"); }
    }
  }
  
  return fav;
}

bool LUtils::saveFavorites(QStringList list){
  bool ok = LUtils::writeFile(QDir::homePath()+"/.lumina/favorites/fav.list", list, true);
  if(ok){ fav = list; } //also save internally in case of rapid write/read of the file
  return ok;
}

bool LUtils::isFavorite(QString path){
  QStringList fav = LUtils::listFavorites();
  for(int i=0; i<fav.length(); i++){
    if(fav[i].endsWith("::::"+path)){ return true; }
  }
  return false;
}

bool LUtils::addFavorite(QString path, QString name){
  //Generate the type of favorite this is
  QFileInfo info(path);
  QString type;
  if(info.isDir()){ type="dir"; }
  else if(info.suffix()=="desktop"){ type="app"; }
  else{ type = LXDG::findAppMimeForFile(path); }
  //Assign a name if none given
  if(name.isEmpty()){ name = info.fileName(); }
  //Now add it to the list
  QStringList favs = LUtils::listFavorites();
  bool found = false;
  for(int i=0; i<favs.length(); i++){
    if(favs[i].endsWith("::::"+path)){ favs[i] = name+"::::"+type+"::::"+path; }
  }
  if(!found){ favs << name+"::::"+type+"::::"+path; }
  return LUtils::saveFavorites(favs);
}

void LUtils::removeFavorite(QString path){
  QStringList fav = LUtils::listFavorites();
  bool changed = false;
  for(int i=0; i<fav.length(); i++){
    if(fav[i].endsWith("::::"+path)){ fav.removeAt(i); i--; changed=true;}
  }
  if(changed){ LUtils::saveFavorites(fav); }
}

void LUtils::upgradeFavorites(int fromoldversionnumber){
  if(fromoldversionnumber <= 8004){ // < pre-0.8.4>, sym-links in the ~/.lumina/favorites dir}
    //Include 0.8.4-devel versions in this upgrade (need to distinguish b/w devel and release versions later somehow)
    QDir favdir(QDir::homePath()+"/.lumina/favorites");
    QFileInfoList symlinks = favdir.entryInfoList(QDir::Files | QDir::Dirs | QDir::System | QDir::NoDotAndDotDot);
    QStringList favfile = LUtils::listFavorites(); //just in case some already exist
    bool newentry = false;
    for(int i=0; i<symlinks.length(); i++){
      if(!symlinks[i].isSymLink()){ continue; } //not a symlink
      QString path = symlinks[i].symLinkTarget();
      QString name = symlinks[i].fileName(); //just use the name of the symlink from the old system
      QString type;
      if(symlinks[i].isDir()){ type = "dir"; }
      else if(name.endsWith(".desktop")){ type = "app"; }
      else{ type = LXDG::findAppMimeForFile(path); }
      //Put the line into the file
      favfile << name+"::::"+type+"::::"+path;
      //Now remove the symlink - obsolete format
      QFile::remove(symlinks[i].absoluteFilePath());
      newentry = true;
    }
    if(newentry){
      LUtils::saveFavorites(favfile);
    }
  } //end check for version <= 0.8.4

}  

void LUtils::LoadSystemDefaults(bool skipOS){
  //Will create the Lumina configuration files based on the current system template (if any)
  qDebug() << "Loading System Defaults";
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
  QRect screenGeom;
  for(int i=0; i<desk->screenCount(); i++){
     if(desk->screenGeometry(i).x()==0){ 
	screen = QString::number(i); 
	screenGeom = desk->screenGeometry(i); 
	break; 
    }
  }
  //Now setup the default "desktopsettings.conf" and "sessionsettings.conf" files
  QStringList deskset, sesset, lopenset;
  
  // -- SESSION SETTINGS --
  QStringList tmp = sysDefaults.filter("session_");
  if(tmp.isEmpty()){ tmp = sysDefaults.filter("session."); }//for backwards compat
  sesset << "[General]"; //everything is in this section
  sesset << "DesktopVersion="+LUtils::LuminaDesktopVersion();
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).toLower().simplified();
    QString istrue = (val=="true") ? "true": "false";
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); } 
    //Now parse the variable and put the value in the proper file   

    if(var=="session_enablenumlock"){ sesset << "EnableNumlock="+ istrue; }
    else if(var=="session_playloginaudio"){ sesset << "PlayStartupAudio="+istrue; }
    else if(var=="session_playlogoutaudio"){ sesset << "PlayLogoutAudio="+istrue; }
    else if(var=="session_default_terminal"){ sesset << "default-terminal="+val; }
    else if(var=="session_default_filemanager"){ 
      sesset << "default-filemanager="+val;
      lopenset << "directory="+val; 
    }
    else if(var=="session_default_webbrowser"){ lopenset << "webbrowser="+val; }
    else if(var=="session_default_email"){ lopenset << "email="+val; }
  }
  if(!lopenset.isEmpty()){ lopenset.prepend("[default]"); } //the session options exist within this set

  // -- DESKTOP SETTINGS --
  //(only works for the primary desktop at the moment)
  tmp = sysDefaults.filter("desktop_");
  if(tmp.isEmpty()){ tmp = sysDefaults.filter("desktop."); }//for backwards compat
  if(!tmp.isEmpty()){deskset << "[desktop-"+screen+"]"; }
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).toLower().simplified();
    QString istrue = (val=="true") ? "true": "false";
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); } 
    //Now parse the variable and put the value in the proper file   
    if(var=="desktop_visiblepanels"){ deskset << "panels="+val; }
    else if(var=="desktop_backgroundfiles"){ deskset << "background\\filelist="+val; }
    else if(var=="desktop_backgroundrotateminutes"){ deskset << "background\\minutesToChange="+val; }
    else if(var=="desktop_plugins"){ deskset << "pluginlist="+val; }
    else if(var=="desktop_generate_icons"){ deskset << "generateDesktopIcons="+istrue; }
  }
  if(!tmp.isEmpty()){ deskset << ""; } //space between sections

  // -- PANEL SETTINGS --
  //(only works for the primary desktop at the moment)
  for(int i=1; i<11; i++){
    QString panvar = "panel"+QString::number(i);
    tmp = sysDefaults.filter(panvar);
    if(!tmp.isEmpty()){deskset << "[panel"+screen+"."+QString::number(i-1)+"]"; }
    for(int i=0; i<tmp.length(); i++){
      if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
      QString var = tmp[i].section("=",0,0).toLower().simplified();
      QString val = tmp[i].section("=",1,1).section("#",0,0).toLower().simplified();
      QString istrue = (val=="true") ? "true": "false";
      //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
      if(var.contains(".")){ var.replace(".","_"); } 
      //Now parse the variable and put the value in the proper file   
      if(var==(panvar+"_pixelsize")){
        //qDebug() << "Panel Size:" << val;
        if(val.endsWith("%h")){ val = QString::number( (screenGeom.height()*val.section("%",0,0).toDouble())/100 ); }//adjust value to a percentage of the height of the screen
        else if(val.endsWith("%w")){ val = QString::number( (screenGeom.width()*val.section("%",0,0).toDouble())/100 ); }//adjust value to a percentage of the width of the screen
        //qDebug() << " -- Adjusted:" << val;
        deskset << "height="+val; 
      }
      else if(var==(panvar+"_autohide")){ deskset << "hidepanel="+istrue; }
      else if(var==(panvar+"_location")){ deskset << "location="+val; }
      else if(var==(panvar+"_plugins")){ deskset << "pluginlist="+val; }
      else if(var==(panvar+"_pinlocation")){ deskset << "pinLocation="+val; }
      else if(var==(panvar+"_edgepercent")){ deskset << "lengthPercent="+val; }
    }
    if(!tmp.isEmpty()){ deskset << ""; } //space between sections
  }

  // -- MENU settings --
  tmp = sysDefaults.filter("menu_");
  if(tmp.isEmpty()){ tmp = sysDefaults.filter("menu."); } //backwards compat
  if(!tmp.isEmpty()){deskset << "[menu]"; }
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).toLower().simplified();
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); } 
    //Now parse the variable and put the value in the proper file   
    if(var=="menu_plugins"){ deskset << "itemlist="+val; }
  }
  if(!tmp.isEmpty()){ deskset << ""; } //space between sections

  // -- FAVORITES --
  tmp = sysDefaults.filter("favorites_");
  if(tmp.isEmpty()){ tmp = sysDefaults.filter("favorites."); }
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); } 
    //Now parse the variable and put the value in the proper file
    qDebug() << "Favorite entry:" << var << val;
    if(var=="favorites_add_ifexists" && QFile::exists(val)){ qDebug() << " - Exists/Adding:"; LUtils::addFavorite(val); }
    else if(var=="favorites_add"){ qDebug() << " - Adding:"; LUtils::addFavorite(val); }
    else if(var=="favorites_remove"){ qDebug() << " - Removing:"; LUtils::removeFavorite(val); }
  }
  
  //Now do any theme settings
  QStringList themesettings = LTHEME::currentSettings(); 
      //List: [theme path, colorspath, iconsname, font, fontsize]
  qDebug() << "Current Theme Color:" << themesettings[1];
  tmp = sysDefaults.filter("theme_");
  if(tmp.isEmpty()){ tmp = sysDefaults.filter("theme."); }
  bool setTheme = !tmp.isEmpty();
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); } 
    //Now parse the variable and put the value in the proper file   
    if(var=="theme_themefile"){ themesettings[0] = val; }
    else if(var=="theme_colorfile"){ themesettings[1] = val; }
    else if(var=="theme_iconset"){ themesettings[2] = val; }
    else if(var=="theme_font"){ themesettings[3] = val; }
    else if(var=="theme_fontsize"){ 
      if(val.endsWith("%")){ val = QString::number( (screenGeom.height()*val.section("%",0,0).toDouble())/100 )+"px"; }
      themesettings[4] = val; 
    }
  }
  //qDebug() << " - Now Color:" << themesettings[1] << setTheme;
  
  //Now double check that the custom theme/color files exist and reset it will the full path as necessary
  if(setTheme){
    QStringList systhemes = LTHEME::availableSystemThemes();
    QStringList syscolors = LTHEME::availableSystemColors();
    //theme file
    qDebug() << "Detected Themes/colors:" << systhemes << syscolors;
    if( !themesettings[0].startsWith("/") || !QFile::exists(themesettings[0]) || !themesettings[1].endsWith(".qss.template")){
      themesettings[0] = themesettings[0].section(".qss",0,0).simplified();
      for(int i=0; i<systhemes.length(); i++){
	 if(systhemes[i].startsWith(themesettings[0]+"::::",Qt::CaseInsensitive)){
	    themesettings[0] = systhemes[i].section("::::",1,1); //Replace with the full path
	    break;
	 }
      }
    }
    //color file
    if( !themesettings[1].startsWith("/") || !QFile::exists(themesettings[1]) || !themesettings[1].endsWith(".qss.colors") ){
      //Remove any extra/invalid extension
      themesettings[1] = themesettings[1].section(".qss",0,0).simplified();
      for(int i=0; i<syscolors.length(); i++){
	 if(syscolors[i].startsWith(themesettings[1]+"::::",Qt::CaseInsensitive)){
	    themesettings[1] = syscolors[i].section("::::",1,1); //Replace with the full path
	    break;
	 }
      }
    }
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
  LUtils::writeFile(setdir+"/lumina-open.conf", lopenset, true);
}
