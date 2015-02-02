//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
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

//==================
//  THEME ENGINE CLASS
//==================
LuminaThemeEngine::LuminaThemeEngine(QApplication *app){
  application=app; //save this pointer for later
  //Make sure to prefer font antialiasing on the application
  QFont tmp = application->font();
  tmp.setStyleStrategy(QFont::PreferAntialias);
  application->setFont(tmp);
  // Now load the theme stylesheet
  QStringList current = LTHEME::currentSettings();
  theme = current[0]; colors=current[1]; icons=current[2]; font=current[3]; fontsize=current[4];
  application->setStyleSheet( LTHEME::assembleStyleSheet(theme, colors, font, fontsize) );
  QIcon::setThemeName(icons); //make sure this sets set within this environment
  watcher = new QFileSystemWatcher(this);
	watcher->addPath( QDir::homePath()+"/.lumina/themesettings.cfg" );
	watcher->addPaths( QStringList() << theme << colors ); //also watch these files for changes
  connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(watcherChange()) );
}

LuminaThemeEngine::~LuminaThemeEngine(){

}

void LuminaThemeEngine::watcherChange(){
  QStringList current = LTHEME::currentSettings();
  application->setStyleSheet( LTHEME::assembleStyleSheet(current[0], current[1], current[3], current[4]) );
	
  if(icons!=current[2]){
    QIcon::setThemeName(current[2]); //make sure this sets set within this environment
    emit updateIcons();
  }
  //Now save this for later checking
  watcher->removePaths( QStringList() << theme << colors );
  theme = current[0]; colors=current[1]; icons=current[2]; font=current[3]; fontsize=current[4];
  watcher->addPaths( QStringList() << theme << colors );
}
