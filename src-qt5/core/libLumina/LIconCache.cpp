//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LIconCache.h"

#include <LuminaOS.h>
#include <LUtils.h>
#include <LuminaXDG.h>

#include <QDir>
#include <QtConcurrent>

LIconCache::LIconCache(QObject *parent) : QObject(parent){
  connect(this, SIGNAL(InternalIconLoaded(QString, QDateTime, QByteArray*)), this, SLOT(IconLoaded(QString, QDateTime, QByteArray*)) );
}

LIconCache::~LIconCache(){


}

LIconCache* LIconCache::instance(){
  static LIconCache cache;
  return &cache;
}

// === PUBLIC ===
//Icon Checks
bool LIconCache::exists(QString icon){
  if(icon.isEmpty()){ return false; }
  if(HASH.contains(icon)){ return true; } //already
  else if(!icon.startsWith("/")){
    //relative path to file (from icon theme?)
    QString path = findFile(icon);
    if(!path.isEmpty() && QFile::exists(path)){ return true; }
  }else{
    //absolute path to file
    return QFile::exists(icon);
  }
  return false;
}

bool LIconCache::isLoaded(QString icon){
  if(icon.isEmpty()){ return false; }
  if(HASH.contains(icon)){
    return !HASH[icon].icon.isNull();
  }
  return false;
}

QString LIconCache::findFile(QString icon){
  if(icon.isEmpty()){ return ""; }
  
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
    QStringList themedeps = LXDG::getIconThemeDepChain(cTheme, paths);
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
    if(QFile::exists(srch[i]+":"+icon+".svg") && !icon.contains("libreoffice") ){
      return QFileInfo(srch[i]+":"+icon+".svg").absoluteFilePath();
    }else if(QFile::exists(srch[i]+":"+icon+".png")){
      return QFileInfo(srch[i]+":"+icon+".png").absoluteFilePath();
    }
  }
  //If still no icon found, look for any image format in the "pixmaps" directory
  if(QFile::exists(LOS::AppPrefix()+"share/pixmaps/"+icon)){
    if(QFileInfo(LOS::AppPrefix()+"share/pixmaps/"+icon).isDir()){ return ""; }
    return (LOS::AppPrefix()+"share/pixmaps/"+icon);
  }else{
    //Need to scan for any close match in the directory
    QDir pix(LOS::AppPrefix()+"share/pixmaps");
    QStringList formats = LUtils::imageExtensions();
    QStringList found = pix.entryList(QStringList() << icon, QDir::Files, QDir::Unsorted);
    if(found.isEmpty()){ found = pix.entryList(QStringList() << icon+"*", QDir::Files, QDir::Unsorted); }
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


void LIconCache::loadIcon(QAbstractButton *button, QString icon, bool noThumb){
  if(icon.isEmpty()){ return; }
  if(isThemeIcon(icon)){
    button->setIcon( iconFromTheme(icon));
    return ;
  }
  //See if the icon has already been loaded into the HASH
  bool needload = !HASH.contains(icon);
  if(!needload){
    if(!noThumb && !HASH[icon].thumbnail.isNull()){ button->setIcon( HASH[icon].thumbnail ); return; }
    else if(!HASH[icon].icon.isNull()){ button->setIcon( HASH[icon].icon ); return; }
  }
  //Need to load the icon
  icon_data idata;
  if(HASH.contains(icon)){ idata = HASH.value(icon); }
  else { idata = createData(icon); }
    idata.pendingButtons << QPointer<QAbstractButton>(button); //save this button for later
  HASH.insert(icon, idata);
  if(needload){ startReadFile(icon, idata.fullpath); }
}

void LIconCache::loadIcon(QAction *action, QString icon, bool noThumb){
  if(icon.isEmpty()){ return; }
  if(isThemeIcon(icon)){
    action->setIcon( iconFromTheme(icon));
    return ;
  }
  //See if the icon has already been loaded into the HASH
  bool needload = !HASH.contains(icon);
  if(!needload){
    if(!noThumb && !HASH[icon].thumbnail.isNull()){ action->setIcon( HASH[icon].thumbnail ); return; }
    else if(!HASH[icon].icon.isNull()){ action->setIcon( HASH[icon].icon ); return; }
  }
  //Need to load the icon
  icon_data idata;
  if(HASH.contains(icon)){ idata = HASH.value(icon); }
  else { idata = createData(icon); }
    idata.pendingActions << QPointer<QAction>(action); //save this button for later
  HASH.insert(icon, idata);
  if(needload){ startReadFile(icon, idata.fullpath); }
}

void LIconCache::loadIcon(QLabel *label, QString icon, bool noThumb){
  if(icon.isEmpty()){ return; }
  if(isThemeIcon(icon)){
    label->setPixmap( iconFromTheme(icon).pixmap(label->sizeHint()) );
    return ;
  }
  //See if the icon has already been loaded into the HASH
  bool needload = !HASH.contains(icon);
  if(!needload){
    if(!noThumb && !HASH[icon].thumbnail.isNull()){ label->setPixmap( HASH[icon].thumbnail.pixmap(label->sizeHint()) ); return; }
    else if(!HASH[icon].icon.isNull()){ label->setPixmap( HASH[icon].icon.pixmap(label->sizeHint()) ); return; }
  }
  //Need to load the icon
  icon_data idata;
  if(HASH.contains(icon)){ idata = HASH.value(icon); }
  else { idata = createData(icon);
    if(idata.fullpath.isEmpty()){ return; } //nothing to do
  }
  idata.pendingLabels << QPointer<QLabel>(label); //save this QLabel for later
  HASH.insert(icon, idata);
  if(needload){ startReadFile(icon, idata.fullpath); }
}

void LIconCache::loadIcon(QMenu *action, QString icon, bool noThumb){
  if(icon.isEmpty()){ return; }
  if(isThemeIcon(icon)){
    action->setIcon( iconFromTheme(icon));
    return ;
  }
  //See if the icon has already been loaded into the HASH
  bool needload = !HASH.contains(icon);
  if(!needload){
    if(!noThumb && !HASH[icon].thumbnail.isNull()){ action->setIcon( HASH[icon].thumbnail ); return; }
    else if(!HASH[icon].icon.isNull()){ action->setIcon( HASH[icon].icon ); return; }
  }
  //Need to load the icon
  icon_data idata;
  if(HASH.contains(icon)){ idata = HASH.value(icon); }
  else { idata = createData(icon); }
    idata.pendingMenus << QPointer<QMenu>(action); //save this button for later
  HASH.insert(icon, idata);
  if(needload){ startReadFile(icon, idata.fullpath); }
}

void LIconCache::clearIconTheme(){
   //use when the icon theme changes to refresh all requested icons
  QStringList keys = HASH.keys();
  for(int i=0; i<keys.length(); i++){
    //remove all relative icons (
    if(!keys.startsWith("/")){ HASH.remove(keys[i]); }
  }
}

QIcon LIconCache::loadIcon(QString icon, bool noThumb){
  if(icon.isEmpty()){ return QIcon(); }
  if(isThemeIcon(icon)){ return iconFromTheme(icon); }

  if(HASH.contains(icon)){
    if(!HASH[icon].icon.isNull()){ return HASH[icon].icon; }
    else if(!HASH[icon].thumbnail.isNull() && !noThumb){ return HASH[icon].thumbnail; }
  }
  //Not loaded yet - need to load it right now
  icon_data idat;
  if(HASH.contains(icon)){ idat = HASH[icon]; }
  else{ idat = createData(icon); }
  if(idat.fullpath.isEmpty()){ return QIcon(); } //non-existant file
  idat.icon = QIcon(idat.fullpath);
  //Now save into the hash and return
  HASH.insert(icon, idat);
  emit IconAvailable(icon);
  return idat.icon;
}

void LIconCache::clearAll(){
  HASH.clear();
}

// === PRIVATE ===
icon_data LIconCache::createData(QString icon){
  icon_data idat;
  //Find the real path of the icon
  if(icon.startsWith("/")){ idat.fullpath = icon; } //already full path
  else{  idat.fullpath = findFile(icon); }
  return idat;
}

QStringList LIconCache::getChildIconDirs(QString path){
//This is a recursive function that returns the absolute path(s) of directories with *.png files
  QDir D(path);
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

QStringList LIconCache::getIconThemeDepChain(QString theme, QStringList paths){
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

void LIconCache::startReadFile(QString id, QString path){
  if(path.endsWith(".svg")){
    //Special handling - need to read QIcon directly to have the SVG icon scale up appropriately
    icon_data idat = HASH[id];
    idat.lastread = QDateTime::currentDateTime();
    idat.icon = QIcon(path);
    for(int i=0; i<idat.pendingButtons.length(); i++){ if(!idat.pendingButtons[i].isNull()){ idat.pendingButtons[i]->setIcon(idat.icon); } }
    idat.pendingButtons.clear();
    for(int i=0; i<idat.pendingLabels.length(); i++){ if(!idat.pendingLabels[i].isNull()){ idat.pendingLabels[i]->setPixmap(idat.icon.pixmap(idat.pendingLabels[i]->sizeHint())); } }
    idat.pendingLabels.clear();
    for(int i=0; i<idat.pendingActions.length(); i++){ if(!idat.pendingActions[i].isNull()){ idat.pendingActions[i]->setIcon(idat.icon); } }
    idat.pendingActions.clear();
    for(int i=0; i<idat.pendingMenus.length(); i++){ if(!idat.pendingMenus[i].isNull()){ idat.pendingMenus[i]->setIcon(idat.icon); } }
    idat.pendingMenus.clear();
    //Now update the hash and let the world know it is available now
    HASH.insert(id, idat);
    this->emit IconAvailable(id);
  }else{
    QtConcurrent::run(this, &LIconCache::ReadFile, this, id, path);
  }
}

void LIconCache::ReadFile(LIconCache *obj, QString id, QString path){
  //qDebug() << "Start Reading File:" << id << path;
  QByteArray *BA = new QByteArray();
  QDateTime cdt = QDateTime::currentDateTime();
  if(!path.isEmpty()){
    QFile file(path);
    if(file.open(QIODevice::ReadOnly)){
      BA->append(file.readAll());
      file.close();
    }
  }
  obj->emit InternalIconLoaded(id, cdt, BA);
}

bool LIconCache::isThemeIcon(QString id){
  return (!id.contains("/") && !id.contains(".") ); //&& !id.contains("libreoffice") );
}

QIcon LIconCache::iconFromTheme(QString id){
  QIcon ico = QIcon::fromTheme(id);
  if(ico.isNull()){
    //icon missing in theme? run the old icon-finder system
    ico = QIcon(findFile(id));
  }
  return ico;
}

// === PRIVATE SLOTS ===
void LIconCache::IconLoaded(QString id, QDateTime sync, QByteArray *data){
  //qDebug() << "Icon Loaded:" << id << HASH.contains(id);
  QPixmap pix;
  bool ok = pix.loadFromData(*data);
   delete data; //no longer used - free this up
  if(!HASH.contains(id)){ return; } //icon loading cancelled - just stop here
  if(!ok){ HASH.remove(id); } //icon data corrupted or unreadable
  else{
    icon_data idat = HASH[id];
    idat.lastread = sync;
    idat.icon.addPixmap(pix);
    if(pix.width() < 64){ idat.icon.addPixmap( pix.scaled( QSize(64,64), Qt::KeepAspectRatio, Qt::SmoothTransformation) ); } //also add a version which has been scaled up a bit
    //Now throw this icon into any pending objects
    for(int i=0; i<idat.pendingButtons.length(); i++){ if(!idat.pendingButtons[i].isNull()){ idat.pendingButtons[i]->setIcon(idat.icon); } }
    idat.pendingButtons.clear();
    for(int i=0; i<idat.pendingLabels.length(); i++){ if(!idat.pendingLabels[i].isNull()){ idat.pendingLabels[i]->setPixmap(pix.scaled(idat.pendingLabels[i]->sizeHint(), Qt::KeepAspectRatio, Qt::SmoothTransformation)); } }
    idat.pendingLabels.clear();
    for(int i=0; i<idat.pendingActions.length(); i++){ if(!idat.pendingActions[i].isNull()){ idat.pendingActions[i]->setIcon(idat.icon); } }
    idat.pendingActions.clear();
    //Now update the hash and let the world know it is available now
    HASH.insert(id, idat);
    this->emit IconAvailable(id);
  }
}
