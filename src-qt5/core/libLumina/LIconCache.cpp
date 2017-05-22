//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LIconCache.h"

#include <LuminaOS.h>

LIconCache::LIconCache(QObject *parent = 0{

}

LIconCache::~LIconCache(){


}

// === PUBLIC ===
//Icon Checks
bool LIconCache::exists(QString icon){
  if(HASH.contains(icon){ return true; } //already
  else if(!icon.startsWith("/")){
    //relative path to file (from icon theme?)
    QString path = findFile(QString icon);
    if(!path.isEmpty() && QFile::exists(path)){ return true; }
  }else{
    //absolute path to file
    return QFile::exists(icon);
  }
  return false;
}

bool LIconCache::isLoaded(QString icon){
  if(HASH.contains(icon){
    return !HASH[icon].icon.isNull();
  }
  return false;
}

QString LIconCache::findFile(QString icon){
//Get the currently-set theme
  QString cTheme = QIcon::themeName();
  if(cTheme.isEmpty()){ 
    QIcon::setThemeName("material-design-light"); 
    cTheme = "material-design-light";	  
  }
  //Make sure the current search paths correspond to this theme
  if( QDir::searchPaths("icontheme").filter("/"+cTheme+"/").isEmpty() ){
    //Need to reset search paths: setup the "icontheme" "material-design-light" and "fallback" sets
    // - Get all the base icon directories
    QStringList paths;
      paths << QDir::homePath()+"/.icons/"; //ordered by priority - local user dirs first
      QStringList xdd = QString(getenv("XDG_DATA_HOME")).split(":");
        xdd << QString(getenv("XDG_DATA_DIRS")).split(":");
        for(int i=0; i<xdd.length(); i++){
          if(QFile::exists(xdd[i]+"/icons")){ paths << xdd[i]+"/icons/"; }
        }
    //Now load all the dirs into the search paths
    QStringList theme, oxy, fall;
    QStringList themedeps = getIconThemeDepChain(cTheme, paths);
    for(int i=0; i<paths.length(); i++){
      theme << getChildIconDirs( paths[i]+cTheme);
      for(int j=0; j<themedeps.length(); j++){ theme << getChildIconDirs(paths[i]+themedeps[j]); }
      oxy << getChildIconDirs(paths[i]+"material-design-light"); //Lumina base icon set
      fall << getChildIconDirs(paths[i]+"hicolor"); //XDG fallback (apps add to this)
    }
    //Now load all the icon theme dependencies in order (Theme1 -> Theme2 -> Theme3 -> Fallback)
    
    //fall << LOS::AppPrefix()+"share/pixmaps"; //always use this as well as a final fallback
    QDir::setSearchPaths("icontheme", theme);
    QDir::setSearchPaths("default", oxy);
    QDir::setSearchPaths("fallback", fall);
    //qDebug() << "Setting Icon Search Paths:" << "\nicontheme:" << theme << "\nmaterial-design-light:" << oxy << "\nfallback:" << fall;
  }
  //Find the icon in the search paths
  QIcon ico;
  QStringList srch; srch << "icontheme" << "default" << "fallback";
  for(int i=0; i<srch.length() && ico.isNull(); i++){
    if(QFile::exists(srch[i]+":"+iconName+".svg") && !iconName.contains("libreoffice") ){
      return QFile(srch[i]+":"+iconName+".svg").absoluteFilePath();
    }else if(QFile::exists(srch[i]+":"+iconName+".png")){
      return QFile(srch[i]+":"+iconName+".png").absoluteFilePath();
    }
  }
  //If still no icon found, look for any image format in the "pixmaps" directory
  if(QFile::exists(LOS::AppPrefix()+"share/pixmaps/"+iconName)){
    return QFile(LOS::AppPrefix()+"share/pixmaps/"+iconName).absoluteFilePath();
  }else{
    //Need to scan for any close match in the directory
    QDir pix(LOS::AppPrefix()+"share/pixmaps");
    QStringList formats = LUtils::imageExtensions();
    QStringList found = pix.entryList(QStringList() << iconName, QDir::Files, QDir::Unsorted);
    if(found.isEmpty()){ found = pix.entryList(QStringList() << iconName+"*", QDir::Files, QDir::Unsorted); }
    //qDebug() << "Found pixmaps:" << found << formats;
    //Use the first one found that is a valid format
    for(int i=0; i<found.length(); i++){
      if( formats.contains(found[i].section(".",-1).toLower()) ){
        return pix.absoluteFilePath(found[i]);
      }
    }
  }
  return ""; //no file found
}


void LIconCache::loadIcon(QAbstractButton *button, QString icon, bool noThumb = false){
  //See if the icon has already been loaded into the HASH
  if(HASH.contains(icon)){
    if(!noThumb && !HASH[icon].thumbnail.isNull()){ button->setIcon( HASH[icon].thumbnail ); return; }
    else if(!HASH[icon].icon.isNull()){ button->setIcon( HASH[icon].icon ); return; }
  }
  //Need to load the icon
  icon_data idata;
  if(HASH.contains(icon)){ idata = HASH.value(icon); }
  else { idata = icon_data createData(QString icon); }
    idata.pendingButtons << button; //save this button for later
  HASH.insert(icon, idata);
  QtConcurrent::run(this, &LIconCache::ReadFile, this, icon, idata.fullpath);
}

void LIconCache::loadIcon(QLabel *label, QString icon, bool noThumb = false){
  //See if the icon has already been loaded into the HASH
  if(HASH.contains(icon)){
    if(!noThumb && !HASH[icon].thumbnail.isNull()){ button->setIcon( HASH[icon].thumbnail ); return; }
    else if(!HASH[icon].icon.isNull()){ button->setIcon( HASH[icon].icon ); return; }
  }
  //Need to load the icon
  icon_data idata;
  if(HASH.contains(icon)){ idata = HASH.value(icon); }
  else { idata = icon_data createData(QString icon); }
  idata.pendingLabels << label; //save this QLabel for later
  HASH.insert(icon, idata);
  QtConcurrent::run(this, &LIconCache::ReadFile, this, icon, idata.fullpath);
}


// === PRIVATE ===
icon_data LIconCache::createData(QString icon){
  icon_data idat;
  //Find the real path of the icon
  if(icon.startsWith("/")){ idat.fullpath = icon; } //already full path
  else{
    idat.fullpath = findFile(icon);
  }
  return idat;
}

QStringList LIconCache::getChildIconDirs(QString path){
//This is a recursive function that returns the absolute path(s) of directories with *.png files
  QDir D(parent);
  QStringList out;
  QStringList dirs = D.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  if(!dirs.isEmpty() && (dirs.contains("32x32") || dirs.contains("scalable")) ){ 
    //Need to sort these directories by image size
    //qDebug() << " - Parent:" << parent << "Dirs:" << dirs;
    for(int i=0; i<dirs.length(); i++){
      if(dirs[i].contains("x")){ dirs[i].prepend( QString::number(10-dirs[i].section("x",0,0).length())+QString::number(10-dirs[i].at(0).digitValue())+"::::"); }
      else if(dirs[i].at(0).isNumber()){dirs[i].prepend( QString::number(10-dirs[i].length())+QString::number(10-dirs[i].at(0).digitValue())+"::::"); }
      else{ dirs[i].prepend( "0::::"); }
    }
    dirs.sort();
    for(int i=0; i<dirs.length(); i++){ dirs[i] = dirs[i].section("::::",1,50); } //chop the sorter off the front again
    //qDebug() << "Sorted:" << dirs;
  }
  QStringList img = D.entryList(QStringList() << "*.png" << "*.svg", QDir::Files | QDir::NoDotAndDotDot, QDir::NoSort);
  if(img.length() > 0){ out << D.absolutePath(); }
  for(int i=0; i<dirs.length(); i++){
    img.clear();
    img = getChildIconDirs(D.absoluteFilePath(dirs[i])); //re-use the old list variable
    if(img.length() > 0){ out << img; }
  }
  return out;
}

QStringList LXDG::getIconThemeDepChain(QString theme, QStringList paths){
  QStringList results;
  for(int i=0; i<paths.length(); i++){
    if( QFile::exists(paths[i]+theme+"/index.theme") ){
      QStringList deps = LUtils::readFile(paths[i]+theme+"/index.theme").filter("Inherits=");
      if(!deps.isEmpty()){
        deps = deps.first().section("=",1,-1).split(";",QString::SkipEmptyParts);
        for(int j=0; j<deps.length(); j++){
          results << deps[j] << getIconThemeDepChain(deps[j],paths);
        }
      }
      break; //found primary theme index file - stop here
    }
  }
  return results;
}

void LIconCache::ReadFile(LIconCache *obj, QString id, QString path){

}

// === PRIVATE SLOTS ===
void LIconCache::IconLoaded(QString id, QDateTime sync, QByteArray *data){

}
