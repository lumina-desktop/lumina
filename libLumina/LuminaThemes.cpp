//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LuminaThemes.h"

#include "LuminaUtils.h"
#include "LuminaOS.h"
#include <QIcon>
#include <QFont>
#include <QDebug>
#include <QObject>

#include <unistd.h>

//Stuff necesary for Qt Cursor Reloads
//#include "qxcbcursor.h" //needed to prod Qt to refresh the mouse cursor theme
//#include <QCursor>

QStringList LTHEME::availableSystemThemes(){ 
  //returns: [name::::path] for each item
  QDir dir(LOS::LuminaShare()+"themes");
  QStringList list = dir.entryList(QStringList() <<"*.qss.template", QDir::Files, QDir::Name);
  for(int i=0; i<list.length(); i++){
    //Format the output entry [<name>::::<fullpath>]
    list[i] = list[i].section(".qss.",0,0)+"::::"+dir.absoluteFilePath(list[i]);
  }
  return list;	
}

QStringList LTHEME::availableLocalThemes(){	//returns: [name::::path] for each item
  QDir dir(QDir::homePath()+"/.lumina/themes");
  QStringList list = dir.entryList(QStringList() <<"*.qss.template", QDir::Files, QDir::Name);
  for(int i=0; i<list.length(); i++){
    //Format the output entry [<name>::::<fullpath>]
    list[i] = list[i].section(".qss.",0,0)+"::::"+dir.absoluteFilePath(list[i]);
  }
  return list;
}

QStringList LTHEME::availableSystemColors(){ 	//returns: [name::::path] for each item
  //returns: [name::::path] for each item
  QDir dir(LOS::LuminaShare()+"colors");
  QStringList list = dir.entryList(QStringList() <<"*.qss.colors", QDir::Files, QDir::Name);
  for(int i=0; i<list.length(); i++){
    //Format the output entry [<name>::::<fullpath>]
    list[i] = list[i].section(".qss.",0,0)+"::::"+dir.absoluteFilePath(list[i]);
  }
  return list;	
}

QStringList LTHEME::availableLocalColors(){ 	//returns: [name::::path] for each item
  QDir dir(QDir::homePath()+"/.lumina/colors");
  QStringList list = dir.entryList(QStringList() <<"*.qss.colors", QDir::Files, QDir::Name);
  for(int i=0; i<list.length(); i++){
    //Format the output entry [<name>::::<fullpath>]
    list[i] = list[i].section(".qss.",0,0)+"::::"+dir.absoluteFilePath(list[i]);
  }
  return list;	
}

QStringList LTHEME::availableSystemIcons(){ 	//returns: [name] for each item
  QStringList paths;
  paths << QDir::homePath()+"/.icons";
  QStringList xdd = QString(getenv("XDG_DATA_HOME")).split(":");
  xdd << QString(getenv("XDG_DATA_DIRS")).split(":");
  for(int i=0; i<xdd.length(); i++){
    if(QFile::exists(xdd[i]+"/icons")){
      paths << xdd[i]+"/icons";	    
    }
  }
  //Now get all the icon themes in these directories
  QStringList themes, tmpthemes;
  QDir dir;
  for(int i=0; i<paths.length(); i++){
    if(dir.cd(paths[i])){
       tmpthemes = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
       for(int j=0; j<tmpthemes.length(); j++){
	 if(tmpthemes[j].startsWith("default")){ continue; }
         if(QFile::exists(dir.absoluteFilePath(tmpthemes[j]+"/index.theme")) ){ themes << tmpthemes[j]; }
       }
    }
  }
  themes.removeDuplicates();
  themes.sort();
  return themes;
}
	
QStringList LTHEME::availableSystemCursors(){	//returns: [name] for each item
  QStringList paths; paths << LOS::SysPrefix()+"lib/X11/icons/" << LOS::AppPrefix()+"lib/X11/icons/";
  QStringList out;
  for(int i=0; i<paths.length(); i++){
    if( !QFile::exists(paths[i]) ){ continue; }
    QDir dir(paths[i]);
    QStringList tmp = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for(int j=0; j<tmp.length(); j++){
      if(QFile::exists(paths[i]+tmp[j]+"/cursors")){
        out << tmp[j]; //good theme - save it to the output list
      }
    }
  }
  return out;
}

//Save a new theme/color file
bool LTHEME::saveLocalTheme(QString name, QStringList contents){
  QString localdir = QDir::homePath()+"/.lumina/themes/";
  if(!QFile::exists(localdir)){  QDir dir; dir.mkpath(localdir); }
  return LUtils::writeFile(localdir+name+".qss.template", contents, true);
}

bool LTHEME::saveLocalColors(QString name, QStringList contents){
  QString localdir = QDir::homePath()+"/.lumina/colors/";
  if(!QFile::exists(localdir)){  QDir dir; dir.mkpath(localdir); }
  return LUtils::writeFile(localdir+name+".qss.colors", contents, true);	
}

//Return the currently selected Theme/Colors/Icons
QStringList LTHEME::currentSettings(){ //returns [theme path, colorspath, iconsname, font, fontsize]
  QStringList out; out << "" << "" << "" << "" << "";
  QStringList settings = LUtils::readFile(QDir::homePath()+"/.lumina/themesettings.cfg");
  for(int i=0; i<settings.length(); i++){
    if(settings[i].startsWith("THEMEFILE=")){ out[0] = settings[i].section("=",1,1).simplified(); }
    else if(settings[i].startsWith("COLORFILE=")){ out[1] = settings[i].section("=",1,1).simplified(); }
    else if(settings[i].startsWith("ICONTHEME=")){ out[2] = settings[i].section("=",1,1).simplified(); }
    else if(settings[i].startsWith("FONTFAMILY=")){ out[3] = settings[i].section("=",1,1).simplified(); }
    else if(settings[i].startsWith("FONTSIZE=")){ out[4] = settings[i].section("=",1,1).simplified(); }
  }
  bool nofile = settings.isEmpty();
  if(out[0].isEmpty() || !QFile::exists(out[0]) ){ out[0] = LOS::LuminaShare()+"themes/Lumina-default.qss.template"; }
  if(out[1].isEmpty() || !QFile::exists(out[1]) ){ out[1] = LOS::LuminaShare()+"colors/Lumina-Glass.qss.colors"; }
  if(out[3].isEmpty()){ out[3] = QFont().defaultFamily(); }
  if(out[4].isEmpty()){ 
    int num = QFont().pointSize(); out[4] = QString::number(num)+"pt"; //Check point size first
    if(num<0){ num = QFont().pixelSize(); out[4] = QString::number(num)+"px";} //Now check pixel size
    if(num<0){ out[4] = "9pt"; } //Now hard-code a fallback (just in case)
  }
  if(nofile){ setCurrentSettings(out[0], out[1], out[2], out[3], out[4]); }
  
  return out;
}

//Return the currently-selected Cursor theme
QString LTHEME::currentCursor(){
  //qDebug() << "Reading Current Cursor Theme:";
  QStringList info = LUtils::readFile(QDir::homePath()+"/.icons/default/index.theme");
  if(info.isEmpty()){ return ""; }
  QString cursor;
  bool insection = false;
  for(int i=0; i<info.length(); i++){
    if(info[i]=="[Icon Theme]"){ insection = true; continue;}
    else if(insection && info[i].startsWith("Inherits=")){
      cursor = info[i].section("=",1,1).simplified();
      break;
    }
  }
  //qDebug() << " - found theme:" << cursor;
  return cursor;
}

  //Change the current Theme/Colors/Icons
bool LTHEME::setCurrentSettings(QString themepath, QString colorpath, QString iconname, QString font, QString fontsize){
  QIcon::setThemeName(iconname);
  //Now save the theme settings file	
  QStringList contents;
	contents << "THEMEFILE="+themepath;
	contents << "COLORFILE="+colorpath;
	contents << "ICONTHEME="+iconname;
	contents << "FONTFAMILY="+font;
	contents << "FONTSIZE="+fontsize;
  bool ok = LUtils::writeFile(QDir::homePath()+"/.lumina/themesettings.cfg", contents, true);

  return ok;
}

//Change the current Cursor Theme
bool LTHEME::setCursorTheme(QString cursorname){
//qDebug() << "Set Cursor Theme:" << cursorname;
  QStringList info = LUtils::readFile(QDir::homePath()+"/.icons/default/index.theme");
    bool insection = false;
    bool changed = false;
    QString newval = "Inherits="+cursorname;
    for(int i=0; i<info.length() && !changed; i++){
      if(info[i]=="[Icon Theme]"){ 
	insection = true;
      }else if( info[i].startsWith("[") && insection){ 
	//Section does not have the setting - add it
	info.insert(i, newval); 
	changed =true;
      }else if( info[i].startsWith("[") ){ 
	insection = false;
      }else if(insection && info[i].startsWith("Inherits=")){
        info[i] = newval; //replace the current setting
        changed = true;
      }
    } //end loop over file contents
    if(!changed){ //Could not change the file contents for some reason
      if(insection){ info << newval; } //end of file while in the section
      else{ info << "[Icon Theme]" << newval; } //entire section missing from file
    }
    //Now save the file
    //qDebug() << "Done saving the cursor:" << info;
    return LUtils::writeFile(QDir::homePath()+"/.icons/default/index.theme", info, true);
}

  //Return the complete stylesheet for a given theme/colors
QString LTHEME::assembleStyleSheet(QString themepath, QString colorpath, QString font, QString fontsize){
  QString stylesheet = LUtils::readFile(themepath).join("\n");
  QStringList colors = LUtils::readFile(colorpath);
  //qDebug() << "Found Theme:" << themepath << stylesheet;
  //qDebug() << "Found Colors:" << colorpath << colors;
  for(int i=0; i<colors.length(); i++){
    if(colors[i].isEmpty() || colors[i].startsWith("#")){ continue; }
    else if(colors[i].startsWith("PRIMARYCOLOR=")){ stylesheet = stylesheet.replace("%%PRIMARYCOLOR%%", colors[i].section("=",1,1).simplified()); }
    else if(colors[i].startsWith("SECONDARYCOLOR=")){ stylesheet = stylesheet.replace("%%SECONDARYCOLOR%%", colors[i].section("=",1,1).simplified()); }
    else if(colors[i].startsWith("HIGHLIGHTCOLOR=")){ stylesheet = stylesheet.replace("%%HIGHLIGHTCOLOR%%", colors[i].section("=",1,1).simplified()); }
    else if(colors[i].startsWith("ACCENTCOLOR=")){ stylesheet = stylesheet.replace("%%ACCENTCOLOR%%", colors[i].section("=",1,1).simplified()); }
    else if(colors[i].startsWith("PRIMARYDISABLECOLOR=")){ stylesheet = stylesheet.replace("%%PRIMARYDISABLECOLOR%%", colors[i].section("=",1,1).simplified()); }
    else if(colors[i].startsWith("SECONDARYDISABLECOLOR=")){ stylesheet = stylesheet.replace("%%SECONDARYDISABLECOLOR%%", colors[i].section("=",1,1).simplified()); }
    else if(colors[i].startsWith("HIGHLIGHTDISABLECOLOR=")){ stylesheet = stylesheet.replace("%%HIGHLIGHTDISABLECOLOR%%", colors[i].section("=",1,1).simplified()); }
    else if(colors[i].startsWith("ACCENTDISABLECOLOR=")){ stylesheet = stylesheet.replace("%%ACCENTDISABLECOLOR%%", colors[i].section("=",1,1).simplified()); }
    else if(colors[i].startsWith("BASECOLOR=")){ stylesheet = stylesheet.replace("%%BASECOLOR%%", colors[i].section("=",1,1).simplified()); }
    else if(colors[i].startsWith("ALTBASECOLOR=")){ stylesheet = stylesheet.replace("%%ALTBASECOLOR%%", colors[i].section("=",1,1).simplified()); }
    else if(colors[i].startsWith("TEXTCOLOR=")){ stylesheet = stylesheet.replace("%%TEXTCOLOR%%", colors[i].section("=",1,1).simplified()); }
    else if(colors[i].startsWith("TEXTDISABLECOLOR=")){ stylesheet = stylesheet.replace("%%TEXTDISABLECOLOR%%", colors[i].section("=",1,1).simplified()); }
    else if(colors[i].startsWith("TEXTHIGHLIGHTCOLOR=")){ stylesheet = stylesheet.replace("%%TEXTHIGHLIGHTCOLOR%%", colors[i].section("=",1,1).simplified()); }
  }
  stylesheet = stylesheet.replace("%%FONT%%", "\""+font+"\"");
  stylesheet = stylesheet.replace("%%FONTSIZE%%", fontsize);
  //qDebug() << "Assembled Style Sheet:\n" << stylesheet;
  return stylesheet;
}
// Extra information about a cursor theme
QStringList LTHEME::cursorInformation(QString name){
  //returns: [Name, Comment, Sample Image File]
  QStringList out; out << "" << "" << ""; //ensure consistent output structure
  QStringList paths; paths << LOS::SysPrefix()+"lib/X11/icons/" << LOS::AppPrefix()+"lib/X11/icons/";
  for(int i=0; i<paths.length(); i++){
    if(QFile::exists(paths[i]+name)){
      if(QFile::exists(paths[i]+name+"/cursors/arrow")){ out[2] = paths[i]+name+"/cursors/arrow"; }
      QStringList info = LUtils::readFile(paths[i]+name+"/index.theme");
      for(int j=info.indexOf("[Icon Theme]"); j<info.length(); j++){
	if(j<0){continue; } //just in case the index function errors out
	if(info[j].startsWith("Name") && info[j].contains("=")){ out[0] = info[j].section("=",1,1).simplified(); }
	else if(info[j].startsWith("Comment") && info[j].contains("=")){ out[1] = info[j].section("=",1,1).simplified(); }
      }
      break; //found the cursor
    }
  }
  return out;
}	

QStringList LTHEME::CustomEnvSettings(){ //view all the key=value settings
  static QStringList info = QStringList();
  static QDateTime lastcheck = QDateTime();
  if(lastcheck.isNull() || lastcheck < QFileInfo(QDir::homePath()+"/.lumina/envsettings.conf").lastModified()){
    lastcheck = QDateTime::currentDateTime();
    info = LUtils::readFile(QDir::homePath()+"/.lumina/envsettings.conf");
  }
  return info;
}

void LTHEME::LoadCustomEnvSettings(){
  //will push the custom settings into the environment (recommended before loading the initial QApplication)
  QStringList info = LTHEME::CustomEnvSettings();
  if(info.isEmpty()){
    //Ensure the file exists, and create it otherwise;
    if(!QFile::exists(QDir::homePath()+"/.lumina/envsettings.conf")){
      LUtils::writeFile(QDir::homePath()+"/.lumina/envsettings.conf", QStringList() << "", true);
    }
  }
  for(int i=0; i<info.length(); i++){
    if(info[i].isEmpty()){ continue; }
    if(info[i].section("=",1,100).isEmpty()){
      unsetenv(info[i].section("=",0,0).toLocal8Bit());
    }else{
      setenv(info[i].section("=",0,0).toLocal8Bit(), info[i].section("=",1,100).simplified().toLocal8Bit(), 1);
    }
  }
	
}

bool LTHEME::setCustomEnvSetting(QString var, QString val){ 
  //variable/value pair (use an empty val to clear it)
  QStringList info = LTHEME::CustomEnvSettings();
  bool changed = false;
  if(!info.filter(var+"=").isEmpty()){
    for(int i=0; i<info.length(); i++){
      //Make sure this is an exact variable match
      if(!info[i].startsWith(var+"=")){ continue; }
        //Found it - replace this line
	info[i] = var+"="+val;
	changed = true;
    }
  }
  if(!changed){ info << var+"="+val; }
  return LUtils::writeFile(QDir::homePath()+"/.lumina/envsettings.conf", info, true); 
}

QString LTHEME::readCustomEnvSetting(QString var){
  QStringList info = LTHEME::CustomEnvSettings().filter(var+"=");
  for(int i=0; i<info.length(); i++){
    if(info[i].startsWith(var+"=")){
      return info[i].section("=",1,100).simplified();
    }
  }
  //If it gets here, no setting found for that variable
  return "";
}

//==================
//  THEME ENGINE CLASS
//==================
LuminaThemeEngine::LuminaThemeEngine(QApplication *app){
  application=app; //save this pointer for later
  lastcheck = QDateTime::currentDateTime(); //
  //Make sure to prefer font antialiasing on the application
  QFont tmp = application->font();
  tmp.setStyleStrategy(QFont::PreferAntialias);
  application->setFont(tmp);
  // Now load the theme stylesheet
  QStringList current = LTHEME::currentSettings();
  theme = current[0]; colors=current[1]; icons=current[2]; font=current[3]; fontsize=current[4];
  cursors = LTHEME::currentCursor();
  application->setStyleSheet( LTHEME::assembleStyleSheet(theme, colors, font, fontsize) );
  QIcon::setThemeName(icons); //make sure this sets set within this environment
  syncTimer = new QTimer(this);
    syncTimer->setSingleShot(true);
    syncTimer->setInterval(500); //wait 1/2 second before re-loading the files
  if(cursors.isEmpty()){
    LTHEME::setCursorTheme("default"); //X11 fallback (always installed?)
    cursors = "default";
  }
  //setenv("XCURSOR_THEME", cursors.toLocal8Bit(),1);
  watcher = new QFileSystemWatcher(this);
	watcher->addPath( QDir::homePath()+"/.lumina/envsettings.conf" );
	watcher->addPath( QDir::homePath()+"/.lumina/themesettings.cfg" );
	watcher->addPaths( QStringList() << theme << colors << QDir::homePath()+"/.icons/default/index.theme" ); //also watch these files for changes
  connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(watcherChange()) );
  connect(syncTimer, SIGNAL(timeout()), this, SLOT(reloadFiles()) );
}

LuminaThemeEngine::~LuminaThemeEngine(){

}

void LuminaThemeEngine::watcherChange(){
  if(syncTimer->isActive()){ syncTimer->stop(); }
  syncTimer->start();
}

void LuminaThemeEngine::reloadFiles(){
  //Check the Theme file/settings
  if(lastcheck < QFileInfo(QDir::homePath()+"/.lumina/themesettings.cfg").lastModified() ){
    QStringList current = LTHEME::currentSettings();
    application->setStyleSheet( LTHEME::assembleStyleSheet(current[0], current[1], current[3], current[4]) );	
    if(icons!=current[2]){
      QIcon::setThemeName(current[2]); //make sure this sets set within this environment
      emit updateIcons();
    }
    //save the settings for comparison later
    theme = current[0]; colors=current[1]; icons=current[2]; font=current[3]; fontsize=current[4];
  }
  //Check the Cursor file/settings
  if(lastcheck < QFileInfo(QDir::homePath()+"/.icons/default/index.theme").lastModified()){
    QString ccurs = LTHEME::currentCursor();
    if(cursors != ccurs){
      emit updateCursors();
      //Might be something we can do automatically here as well - since we have the QApplication handy
      // - Note: setting/unsetting an override cursor does not update the current cursor bitmap
      // Qt created a background database/hash/mapping of the theme pixmaps on startup 
      //   So Qt itself needs to be prodded to update that mapping
      /*QXcbCursor::cursorThemePropertyChanged( \
	  new QXcbVirtualDesktop(QX11Info::connection(), application->screen()->handle(), QX11Info::appScreen()),
	  ccurs.toData(), QVariant("Inherits"), NULL);*/
      //QCursorData::cleanup();
      //QCursorData::initialize();
      //setenv("XCURSOR_THEME", ccurs.toLocal8Bit(),1);
    }
    cursors = ccurs;
  }
  
  
  //Environment Changes
  if( lastcheck < QFileInfo(QDir::homePath()+"/.lumina/envsettings.conf").lastModified()){
    LTHEME::LoadCustomEnvSettings();
    emit EnvChanged();
  }
  lastcheck = QDateTime::currentDateTime();
  
  //Now update the watched files to ensure nothing is missed
  watcher->removePaths( QStringList() << theme << colors << QDir::homePath()+"/.icons/default/index.theme" << QDir::homePath()+"/.lumina/envsettings.conf");
  watcher->addPaths( QStringList() << theme << colors << QDir::homePath()+"/.icons/default/index.theme" << QDir::homePath()+"/.lumina/envsettings.conf");
}
