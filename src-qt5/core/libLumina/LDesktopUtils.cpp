//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LDesktopUtils.h"

#include <QDesktopWidget>
#include <QApplication>
#include <QScreen>
#include <QSettings>

#include "LuminaThemes.h"

QString LDesktopUtils::LuminaDesktopVersion(){
  QString ver = "1.6.2";
  #ifdef GIT_VERSION
  ver.append( QString(" (Git Revision: %1)").arg(GIT_VERSION) );
  #endif
  return ver;
}

QString LDesktopUtils::LuminaDesktopBuildDate(){
  #ifdef BUILD_DATE
  return BUILD_DATE;
  #endif
  return "";
}

//Various function for finding valid QtQuick plugins on the system
bool LDesktopUtils::validQuickPlugin(QString ID){
  return ( !LDesktopUtils::findQuickPluginFile(ID).isEmpty() );
}

QString LDesktopUtils::findQuickPluginFile(QString ID){
  if(ID.startsWith("quick-")){ ID = ID.section("-",1,50); } //just in case
  //Give preference to any user-supplied plugins (overwrites for system plugins)
  QString path = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/quickplugins/quick-"+ID+".qml";
  if( QFile::exists(path) ){return path; }
  path = LOS::LuminaShare()+"quickplugins/quick-"+ID+".qml";
  if( QFile::exists(path) ){return path; }
  return ""; //could not be found
}

QStringList LDesktopUtils::listQuickPlugins(){
  QDir dir(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/quickplugins");
  QStringList files = dir.entryList(QStringList() << "quick-*.qml", QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
  dir.cd(LOS::LuminaShare()+"quickplugins");
  files << dir.entryList(QStringList() << "quick-*.qml", QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
  for(int i=0; i<files.length(); i++){
    files[i] = files[i].section("quick-",1,100).section(".qml",0,0); //just grab the ID out of the middle of the filename
  }
  files.removeDuplicates();
  //qDebug() << "Found Quick Plugins:" << files;
  return files;
}

QStringList LDesktopUtils::infoQuickPlugin(QString ID){ //Returns: [Name, Description, Icon]
  //qDebug() << "Find Quick Info:" << ID;
  QString path = findQuickPluginFile(ID);
  //qDebug() << " - path:" << path;
  if(path.isEmpty()){ return QStringList(); } //invalid ID
  QStringList contents = LUtils::readFile(path);
  if(contents.isEmpty()){ return QStringList(); } //invalid file (unreadable)
  contents = contents.filter("//").filter("=").filter("Plugin"); //now just grab the comments
  //qDebug() << " - Filtered Contents:" << contents;
  QStringList info; info << "" << "" << "";
  for(int i=0; i<contents.length(); i++){
    if(contents[i].contains("Plugin-Name=")){ info[0] = contents[i].section("Plugin-Name=",1,1).simplified(); }
    else if(contents[i].contains("Plugin-Description=")){ info[1] = contents[i].section("Plugin-Description=",1,1).simplified(); }
    else if(contents[i].contains("Plugin-Icon=")){ info[2] = contents[i].section("Plugin-Icon=",1,1).simplified(); }
  }
  if(info[0].isEmpty()){ info[0]=ID; }
  if(info[2].isEmpty()){ info[2]="preferences-plugin"; }
  //qDebug() << " - info:" << info;
  return info;
}

QStringList LDesktopUtils::listFavorites(){
  //static QDateTime lastRead;
  QStringList fav;
  //QDateTime cur = QDateTime::currentDateTime();
  //if(lastRead.isNull() || fav.isEmpty() || lastRead<QFileInfo( QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/favorites.list").lastModified()){
    fav = LUtils::readFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/favorites.list");
    fav.removeAll(""); //remove any empty lines
    fav.removeDuplicates();
    //lastRead = cur;
  //}
  return fav;
}

bool LDesktopUtils::saveFavorites(QStringList list){
  list.removeDuplicates();
  //qDebug() << "Save Favorites:" << list;
  bool ok = LUtils::writeFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/favorites.list", list, true);
  //if(ok){ fav = list; } //also save internally in case of rapid write/read of the file
  return ok;
}

bool LDesktopUtils::isFavorite(QString path){
  QStringList fav = LDesktopUtils::listFavorites();
  for(int i=0; i<fav.length(); i++){
    if(fav[i].endsWith("::::"+path)){ return true; }
  }
  return false;
}

bool LDesktopUtils::addFavorite(QString path, QString name){
  //Generate the type of favorite this is
  QFileInfo info(path);
  QString type;
  if(info.isDir()){ type="dir"; }
  else if(info.suffix()=="desktop"){ type="app"; }
  else{ type = LXDG::findAppMimeForFile(path); }
  //Assign a name if none given
  if(name.isEmpty()){ name = info.fileName(); }
  //qDebug() << "Add Favorite:" << path << type << name;
  //Now add it to the list
  QStringList favs = LDesktopUtils::listFavorites();
  //qDebug() << "Current Favorites:" << favs;
  bool found = false;
  for(int i=0; i<favs.length(); i++){
    if(favs[i].endsWith("::::"+path)){ favs[i] = name+"::::"+type+"::::"+path; found = true; }
  }
  if(!found){ favs << name+"::::"+type+"::::"+path; }
  return LDesktopUtils::saveFavorites(favs);
}

void LDesktopUtils::removeFavorite(QString path){
  QStringList fav = LDesktopUtils::listFavorites();
  bool changed = false;
  for(int i=0; i<fav.length(); i++){
    if(fav[i].endsWith("::::"+path)){ fav.removeAt(i); i--; changed=true;}
  }
  if(changed){ LDesktopUtils::saveFavorites(fav); }
}

void LDesktopUtils::upgradeFavorites(int){ //fromoldversionnumber
  //NOTE: Version number syntax: <major>*1000000 + <minor>*1000 + <revision>
  // Example: 1.2.3 -> 1002003
}

void LDesktopUtils::LoadSystemDefaults(bool skipOS){
  //Will create the Lumina configuration files based on the current system template (if any)
  qDebug() << "Loading System Defaults";
  //Ensure that the settings directory exists
  QString setdir = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop";
  if(!QFile::exists(setdir)){
    QDir dir;
    dir.mkpath(setdir);
  }

  bool skipmime = QFile::exists( QString(getenv("XDG_CONFIG_HOME"))+"/lumina-mimapps.list" );
  //qDebug() << " - Skipping mimetype default apps" << skipmime;
  QStringList sysDefaults;
  if(!skipOS){ sysDefaults = LUtils::readFile(LOS::AppPrefix()+"etc/luminaDesktop.conf"); }
  if(sysDefaults.isEmpty() && !skipOS){ sysDefaults = LUtils::readFile(LOS::AppPrefix()+"etc/luminaDesktop.conf.dist"); }
  if(sysDefaults.isEmpty() && !skipOS) { sysDefaults = LUtils::readFile(LOS::SysPrefix()+"etc/luminaDesktop.conf"); }
  if(sysDefaults.isEmpty() && !skipOS){ sysDefaults = LUtils::readFile(LOS::SysPrefix()+"etc/luminaDesktop.conf.dist"); }
  if(sysDefaults.isEmpty() && !skipOS) { sysDefaults = LUtils::readFile(L_ETCDIR+"/luminaDesktop.conf"); }
  if(sysDefaults.isEmpty() && !skipOS){ sysDefaults = LUtils::readFile(L_ETCDIR+"/luminaDesktop.conf.dist"); }
  if(sysDefaults.isEmpty()){ sysDefaults = LUtils::readFile(LOS::LuminaShare()+"luminaDesktop.conf"); }
  //Find the number of the left-most desktop screen
  QString screen = "0";
  QRect screenGeom;
  QList<QScreen*> screens = QGuiApplication::screens();
  QList<QScreen*>::const_iterator it;
  int i = 0;
  for(it = screens.constBegin(); it != screens.constEnd(); ++it, ++i) {
     if((*it)->availableGeometry().x()==0) {
       screen = i;
       screenGeom = (*it)->availableGeometry();
       break;
    }
  }
  //Now setup the default "desktopsettings.conf" and "sessionsettings.conf" files
  QStringList deskset, sesset;//, lopenset;

  // -- SESSION SETTINGS --
  QStringList tmp = sysDefaults.filter("session_");
  if(tmp.isEmpty()){ tmp = sysDefaults.filter("session."); }//for backwards compat
  sesset << "[General]"; //everything is in this section
  sesset << "DesktopVersion="+LDesktopUtils::LuminaDesktopVersion();
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
    if(val.isEmpty()){ continue; }
    QString istrue = (val.toLower()=="true") ? "true": "false";
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); }
    //Now parse the variable and put the value in the proper file

    if(var.contains("_default_")){ val = LUtils::AppToAbsolute(val); } //got an application/binary
     //Special handling for values which need to exist first
    if(var.endsWith("_ifexists") ){
      var = var.remove("_ifexists"); //remove this flag from the variable
      //Check if the value exists (absolute path only)
      val = LUtils::AppToAbsolute(val);
      //qDebug() << "Checking if favorite app exists:" << val;
      if(!QFile::exists(val)){ continue; } //skip this line - value/file does not exist
    }

    //Parse/save the value
    QString sset; //temporary strings
    if(var=="session_enablenumlock"){ sset = "EnableNumlock="+ istrue; }
    else if(var=="session_playloginaudio"){ sset = "PlayStartupAudio="+istrue; }
    else if(var=="session_playlogoutaudio"){ sset = "PlayLogoutAudio="+istrue; }
    else if(var=="session_default_terminal" && !skipmime){
      LXDG::setDefaultAppForMime("application/terminal", val);
      //sset = "default-terminal="+val;
    }else if(var=="session_default_filemanager" && !skipmime){
      LXDG::setDefaultAppForMime("inode/directory", val);
      //sset = "default-filemanager="+val;
      //loset = "directory="+val;
    }else if(var=="session_default_webbrowser" && !skipmime){
      //loset = "webbrowser="+val;
      LXDG::setDefaultAppForMime("x-scheme-handler/http", val);
      LXDG::setDefaultAppForMime("x-scheme-handler/https", val);
    }else if(var=="session_default_email" && !skipmime){
      LXDG::setDefaultAppForMime("application/email",val);
      //loset = "email="+val;
    }
    //Put the line into the file (overwriting any previous assignment as necessary)
    /*if(!loset.isEmpty()){
      int index = lopenset.indexOf(QRegExp(loset.section("=",0,0)+"=*", Qt::CaseSensitive, QRegExp::Wildcard));
      qDebug() << "loset line:" << loset << index << lopenset;
      if(index<0){ lopenset << loset; } //new line
      else{ lopenset[index] = loset; } //overwrite the other line
    }*/
    if(!sset.isEmpty()){
      int index = sesset.indexOf(QRegExp(sset.section("=",0,0)+"=*", Qt::CaseSensitive, QRegExp::Wildcard));
      if(index<0){ sesset << sset; } //new line
      else{ sesset[index] = sset; } //overwrite the other line
    }
  }
  //if(!lopenset.isEmpty()){ lopenset.prepend("[default]"); } //the session options exist within this set

 // -- MIMETYPE DEFAULTS --
  tmp = sysDefaults.filter("mime_default_");
  for(int i=0; i<tmp.length()  && !skipmime; i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
    qDebug() << "Mime entry:" << var << val;
    if(val.isEmpty()){ continue; }
    QString istrue = (val.toLower()=="true") ? "true": "false";
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); }
    //Now parse the variable and put the value in the proper file
     //Special handling for values which need to exist first
    if(var.endsWith("_ifexists") ){
      var = var.remove("_ifexists"); //remove this flag from the variable
      val = LUtils::AppToAbsolute(val);
      //qDebug() << "Checking if Mime app exists:" << val;
      //Check if the value exists (absolute path only)
      if(!QFile::exists(val)){ continue; } //skip this line - value/file does not exist
    }
    //Now turn this variable into the mimetype only
    var = var.section("_default_",1,-1);
    //qDebug() << " - Set Default Mime:" << var << val;
    LXDG::setDefaultAppForMime(var, val);
  }

  // -- DESKTOP SETTINGS --
  QString deskID = QApplication::primaryScreen()->name();
  //(only works for the primary desktop at the moment)
  tmp = sysDefaults.filter("desktop_");
  if(tmp.isEmpty()){ tmp = sysDefaults.filter("desktop."); }//for backwards compat
  if(!tmp.isEmpty()){deskset << "[desktop-"+deskID+"]"; }
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
    if(val.isEmpty()){ continue; }
    QString istrue = (val.toLower()=="true") ? "true": "false";
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
    if(!tmp.isEmpty()){deskset << "[panel_"+deskID+"."+QString::number(i-1)+"]"; }
    for(int i=0; i<tmp.length(); i++){
      if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
      QString var = tmp[i].section("=",0,0).toLower().simplified();
      QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
      if(val.isEmpty()){ continue; }
      QString istrue = (val.toLower()=="true") ? "true": "false";
      //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
      if(var.contains(".")){ var.replace(".","_"); }
      //Now parse the variable and put the value in the proper file
      if(var==(panvar+"_pixelsize")){
        //qDebug() << "Panel Size:" << val;
	if(val.contains("%")){
	  QString last = val.section("%",1,1).toLower(); //last character
	  val = val.section("%",0,0);
          if(last=="h"){ val = QString::number( qRound(screenGeom.height()*val.toDouble())/100 ); }//adjust value to a percentage of the height of the screen
          else if(last=="w"){ val = QString::number( qRound(screenGeom.width()*val.toDouble())/100 ); }//adjust value to a percentage of the width of the screen
        }
        //qDebug() << " -- Adjusted:" << val;
        deskset << "height="+val;
      }
      else if(var==(panvar+"_autohide")){ deskset << "hidepanel="+istrue; }
      else if(var==(panvar+"_location")){ deskset << "location="+val.toLower(); }
      else if(var==(panvar+"_plugins")){ deskset << "pluginlist="+val; }
      else if(var==(panvar+"_pinlocation")){ deskset << "pinLocation="+val.toLower(); }
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
    QString var = tmp[i].section("=",0,0).simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).toLower().simplified();
    if(val.isEmpty()){ continue; }
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
    //qDebug() << "Found Favorite Entry:" << tmp[i];
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); }
    //Now parse the variable and put the value in the proper file
    qDebug() << "Favorite entry:" << var << val;
    val = LUtils::AppToAbsolute(val); //turn any relative files into absolute
    if(var=="favorites_add_ifexists" && QFile::exists(val)){ qDebug() << " - Exists/Adding:" << val; LDesktopUtils::addFavorite(val); }
    else if(var=="favorites_add"){ qDebug() << " - Adding:"; LDesktopUtils::addFavorite(val); }
    else if(var=="favorites_remove"){ qDebug() << " - Removing:"; LDesktopUtils::removeFavorite(val); }
  }

  tmp = sysDefaults.filter("desktoplinks_");
  QString desktopFolder = LUtils::standardDirectory(LUtils::Desktop);
  desktopFolder.append("/");
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
    val = LUtils::AppToAbsolute(val); //turn any relative files into absolute
    if(var=="desktoplinks_add" && QFile::exists(val) && !QFile::exists(desktopFolder+val.section("/",-1)) ){ QFile::link(val, desktopFolder+val.section("/",-1)); }
  }

  // -- QUICKLAUNCH --
  tmp = sysDefaults.filter("quicklaunch_");
  if(tmp.isEmpty()){ tmp = sysDefaults.filter("quicklaunch."); }
  QStringList quickL;
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); }
    //Now parse the variable and put the value in the proper file
    val = LUtils::AppToAbsolute(val); //turn any relative files into absolute
    if(var=="quicklaunch_add_ifexists" && QFile::exists(val)){ quickL << val; }
    else if(var=="quicklaunch_add"){ quickL << val; }
  }
  if(!quickL.isEmpty()){
    if(sesset.isEmpty()){ sesset << "[General]"; } //everything is in this section
    sesset << "QuicklaunchApps="+quickL.join(", ");
  }

  //Now do any theme settings
  QStringList themesettings = LTHEME::currentSettings();
      //List: [theme path, colorspath, iconsname, font, fontsize]
  //qDebug() << "Current Theme Color:" << themesettings[1];
  tmp = sysDefaults.filter("theme_");
  if(tmp.isEmpty()){ tmp = sysDefaults.filter("theme."); }
  bool setTheme = !tmp.isEmpty();
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
    if(val.isEmpty()){ continue; }
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); }
    //Now parse the variable and put the value in the proper file
    if(var=="theme_themefile"){ themesettings[0] = val; }
    else if(var=="theme_styles"){ LTHEME::setCurrentStyles( val.split(",",QString::SkipEmptyParts) ); }
    else if(var=="theme_colorfile"){ themesettings[1] = val; }
    else if(var=="theme_iconset"){ themesettings[2] = val; }
    else if(var=="theme_font"){ themesettings[3] = val; }
    else if(var=="theme_fontsize"){
      if(val.endsWith("%")){ val = QString::number( (screenGeom.height()*val.section("%",0,0).toDouble())/100 )+"px"; }
      themesettings[4] = val;
    }
    else if(var=="theme_fluxbox"){
      QString fbinit = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/fluxbox-init";
      QStringList contents = LUtils::readFile(fbinit);
      int index = contents.indexOf( contents.filter("session.styleFile:").join("") );
      QString line = "session.styleFile:\t"+val;
      if(index>=0){ contents[index] = line; }
      else{ contents << line; }
      LUtils::writeFile(fbinit, contents, true);
    }
  }
  //qDebug() << " - Now Color:" << themesettings[1] << setTheme;

  //Now double check that the custom theme/color files exist and reset it will the full path as necessary
  if(setTheme){
    QStringList systhemes = LTHEME::availableSystemThemes();
    QStringList syscolors = LTHEME::availableSystemColors();
    //theme file
    //qDebug() << "Detected Themes/colors:" << systhemes << syscolors;
    if( !themesettings[0].startsWith("/") || !QFile::exists(themesettings[0]) || !themesettings[0].endsWith(".qss")){
      themesettings[0] = themesettings[0].section(".qss",0,0).simplified();
      for(int i=0; i<systhemes.length(); i++){
	 if(systhemes[i].startsWith(themesettings[0]+"::::",Qt::CaseInsensitive)){
	    themesettings[0] = systhemes[i].section("::::",1,1); //Replace with the full path
	    break;
	 }
      }
    }
    //color file
    if( !themesettings[1].startsWith("/") || !QFile::exists(themesettings[1]) || !themesettings[1].endsWith(".conf") ){
      //Remove any extra/invalid extension
      themesettings[1] = themesettings[1].section(".conf",0,0).simplified();
      for(int i=0; i<syscolors.length(); i++){
	 if(syscolors[i].startsWith(themesettings[1]+"::::",Qt::CaseInsensitive)){
	    themesettings[1] = syscolors[i].section("::::",1,1); //Replace with the full path
	    break;
	 }
      }
    }

  }
  //qDebug() << " - Final Theme Color:" << themesettings[1];

  //Now save the settings files
  if(setTheme){
    LTHEME::setCurrentSettings( themesettings[0], themesettings[1], themesettings[2], themesettings[3], themesettings[4]);
  }
  LUtils::writeFile(setdir+"/sessionsettings.conf", sesset, true);
  LUtils::writeFile(setdir+"/desktopsettings.conf", deskset, true);

  //Now run any extra config scripts or utilities as needed
  tmp = sysDefaults.filter("usersetup_run");
  if(tmp.isEmpty()){ tmp = sysDefaults.filter("usersetup.run"); }
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); }
    //Now parse the variable and put the value in the proper file
    if(var=="usersetup_run"){
      qDebug() << "Running user setup command:" << val;
      QProcess::execute(val);
    }
  }

}

bool LDesktopUtils::checkUserFiles(QString lastversion, QString currentversion){
  //WARNING: Make sure you create a QApplication instance before calling this function!!!

  //internal version conversion examples:
  //  [1.0.0 -> 1000000], [1.2.3 -> 1002003], [0.6.1 -> 6001]
  //returns true if something changed
  int oldversion = LDesktopUtils::VersionStringToNumber(lastversion);
  int nversion = LDesktopUtils::VersionStringToNumber(currentversion);
  bool newversion =  ( oldversion < nversion ); //increasing version number
  bool newrelease = ( lastversion.contains("-devel", Qt::CaseInsensitive) && QApplication::applicationVersion().contains("-release", Qt::CaseInsensitive) ); //Moving from devel to release
  bool firstrun = false;
  QString confdir = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/";

  //Check the fluxbox configuration files
  QString dset = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/";
  if(!QFile::exists(dset+"fluxbox-init")){
    firstrun = true;
  }
  bool fluxcopy = false;
  if(!QFile::exists(dset+"fluxbox-init")){ fluxcopy=true; }
  else if(!QFile::exists(dset+"fluxbox-keys")){fluxcopy=true; }
  else if(oldversion < 60){ fluxcopy=true; qDebug() << "Current fluxbox settings obsolete: Re-implementing defaults"; }
  if(fluxcopy){
    qDebug() << "Copying default fluxbox configuration files";
    if(QFile::exists(dset+"fluxbox-init")){ QFile::remove(dset+"fluxbox-init"); }
    if(QFile::exists(dset+"fluxbox-keys")){ QFile::remove(dset+"fluxbox-keys"); }
    QString finit = LUtils::readFile(LOS::LuminaShare()+"fluxbox-init-rc").join("\n");
     finit.replace("${XDG_CONFIG_HOME}", QString(getenv("XDG_CONFIG_HOME")));
     LUtils::writeFile(dset+"fluxbox-init", finit.split("\n"));
    QFile::copy(LOS::LuminaShare()+"fluxbox-keys", dset+"fluxbox-keys");
    QFile::setPermissions(dset+"fluxbox-init", QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadOther | QFile::ReadGroup);
    QFile::setPermissions(dset+"fluxbox-keys", QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadOther | QFile::ReadGroup);
  }

  //Check for the desktop settings file
  dset = confdir+"desktopsettings.conf";
  if(!QFile::exists(dset) || oldversion < 5000){
    if( oldversion < 100000 && nversion>=100000 ){ system("rm -rf ~/.lumina"); qDebug() << "Current desktop settings obsolete: Re-implementing defaults"; }
    else{ firstrun = true; }
    LDesktopUtils::LoadSystemDefaults();
  }
  //Convert the favorites framework as necessary (change occured with 0.8.4)
  if(newversion || newrelease){
    LDesktopUtils::upgradeFavorites(oldversion);
  }
  //Convert from the old desktop numbering system to the new one (change occured with 1.0.1)
  if(oldversion<=1000001){
    QStringList DS = LUtils::readFile(dset);
    QList<QScreen*> screens = QApplication::screens();
    for(int i=0; i<DS.length(); i++){
      if(!DS[i].startsWith("[")){ continue; }
      if(DS[i].startsWith("[desktop-")){
        bool ok = false;
        int num = DS[i].section("desktop-",-1).section("]",0,0).toInt(&ok);
        if(num>=0 && ok && num< screens.length()){
          //This one needs to be converted
          DS[i] = "[desktop-"+screens[num]->name()+"]";
        }
      }else if(DS[i].startsWith("[panel")){
        bool ok = false;
        int num = DS[i].section("panel",-1).section(".",0,0).toInt(&ok);
        if(num>=0 && ok && num< screens.length()){
          //This one needs to be converted
         QString rest = DS[i].section(".",1,-1); //everything after the desktop number in the current setting
          DS[i] = "[panel_"+screens[num]->name()+"."+rest;
        }
      }
    }
    LUtils::writeFile(dset, DS, true);
  }
  if(oldversion<1003004){
    //Lumina 1.3.4 - Migrate theme settings from old format to the new theme engine format
    QString themefile = QString(getenv("XDG_CONFIG_HOME"))+"/lthemeengine/lthemeengine.conf";
    if(!QFile::exists(themefile)){
      QDir dir;
      dir.mkpath(themefile.section("/",0,-2)); //make sure the main directory exists first
      //Need to migrate theme settings from the old location to the new one
      QSettings newtheme(themefile, QSettings::NativeFormat);
      qDebug() << "Migrating Theme settings:" << newtheme.fileName();
      QStringList oldtheme = LUtils::readFile( QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/themesettings.cfg" );
      //Find the system install location for the theme engine for use later
      QString enginedir = LOS::LuminaShare()+"/../lthemeengine/";
      //Find/match the icon theme
      QString tmp = oldtheme.filter("ICONTHEME=").join("\n").section("=",1,-1).section("\n",0,0).simplified();
      if(tmp.isEmpty()){ tmp = "material-design-light"; } //unknown Icon theme - use the default "light" version
      newtheme.setValue("Appearance/icon_theme",tmp);
      //Quick detect/adjust of the tone of the color theme based on the icons/colors (no 1-to-1 color theme matching between systems)
      bool isdarktheme = tmp.contains("dark");
      isdarktheme = isdarktheme || oldtheme.filter("COLORFILE=").join("\n").section("=",1,-1).section("\n",0,0).contains("DarkGlass");
      //Quick adjust for the material-design icon theme to make it match the current dark/light theme
      if(tmp.contains("material-design")){
        newtheme.setValue("Appearance/icon_theme", QString("material-design-")+ (isdarktheme ? "dark" : "light") );
      }
      if(isdarktheme){
         newtheme.setValue("Appearance/custom_palette", true);
         newtheme.setValue("Appearance/color_scheme_path", enginedir+"colors/darker.conf");
         newtheme.setValue("Interface/desktop_stylesheets", QStringList() << enginedir+"desktop_qss/DarkGlass.qss");
      }else{
         newtheme.setValue("Appearance/custom_palette", true);
         newtheme.setValue("Appearance/color_scheme_path", enginedir+"colors/airy.conf");
         newtheme.setValue("Interface/desktop_stylesheets", QStringList() << enginedir+"desktop_qss/Glass.qss");
      }
      newtheme.setValue("Appearance/style", "Fusion");
      newtheme.setValue("Interface/stylesheets", QStringList() << enginedir+"qss/tooltip-simple.qss" << enginedir+"qss/scrollbar-simple.qss" << enginedir+"qss/sliders-simple.qss" << enginedir+"qss/traynotification-simple.qss");
      newtheme.sync(); //flush this to file right now
    } //end check for theme file existance
  }

  if(firstrun){ qDebug() << "First time using Lumina!!"; }
  return (firstrun || newversion || newrelease);
}

int LDesktopUtils::VersionStringToNumber(QString version){
  version = version.section("_",0,0).section("-",0,0); //trim any extra labels off the end
  int maj, mid, min; //major/middle/minor version numbers (<Major>.<Middle>.<Minor>)
  maj = mid = min = 0;
  bool ok = true;
  maj = version.section(".",0,0).toInt(&ok);
  if(ok){ mid = version.section(".",1,1).toInt(&ok); }else{ maj = 0; }
  if(ok){ min = version.section(".",2,2).toInt(&ok); }else{ mid = 0; }
  if(!ok){ min = 0; }
  //Now assemble the number
  //NOTE: This format allows numbers to be anywhere from 0->999 without conflict
  return (maj*1000000 + mid*1000 + min);
}

void LDesktopUtils::MigrateDesktopSettings(QSettings *settings, QString fromID, QString toID){
  //desktop-ID
  QStringList keys = settings->allKeys();
  QStringList filter = keys.filter("desktop-"+fromID+"/");
  for(int i=0; i<filter.length(); i++){
    settings->setValue("desktop-"+toID+"/"+filter[i].section("/",1,-1), settings->value(filter[i]));
    settings->remove(filter[i]);
  }
  //panel_ID.[number]
  filter = keys.filter("panel_"+fromID+".");
  for(int i=0; i<filter.length(); i++){
    settings->setValue("panel_"+toID+"."+ filter[i].section("/",0,0).section(".",-1)+"/"+filter[i].section("/",1,-1), settings->value(filter[i]));
    settings->remove(filter[i]);
  }
}
