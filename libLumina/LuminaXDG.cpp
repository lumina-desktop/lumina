//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LuminaXDG.h"

static QStringList mimeglobs;
static qint64 mimechecktime;

//==== LXDG Functions ====
XDGDesktop LXDG::loadDesktopFile(QString filePath, bool& ok){
  //Create the outputs
  ok=false;
  XDGDesktop DF;
    DF.isHidden=false;
    DF.useTerminal=false;
    DF.startupNotify=false;
    DF.type = XDGDesktop::BAD;
    DF.filePath = filePath;
  //Check input file path validity
  QFile file(filePath);
  if(!file.exists()){ return DF; } //invalid file
  //Get the current localization code
  QString lang = QLocale::system().name(); //lang code
  //Open the file
  if(!file.open(QIODevice::Text | QIODevice::ReadOnly)){
    return DF;  
  }
  QTextStream os(&file);
  //Read in the File
  while(!os.atEnd()){
    QString line = os.readLine();
    //Now parse out the file
    line = line.simplified();
    QString var = line.section("=",0,0).simplified();
    QString loc = var.section("[",1,1).section("]",0,0).simplified(); // localization
    var = var.section("[",0,0).simplified(); //remove the localization
    QString val = line.section("=",1,1).simplified();
    //-------------------
    if(var=="Name"){ 
      if(DF.name.isEmpty() && loc.isEmpty()){ DF.name = val; }
      else if(loc == lang){ DF.name = val; }
    }else if(var=="GenericName"){ 
      if(DF.genericName.isEmpty() && loc.isEmpty()){ DF.genericName = val; }
      else if(loc == lang){ DF.genericName = val; }
    }else if(var=="Comment"){ 
      if(DF.comment.isEmpty() && loc.isEmpty()){ DF.comment = val; }
      else if(loc == lang){ DF.comment = val; }
    }else if(var=="Icon"){ 
      if(DF.icon.isEmpty() && loc.isEmpty()){ DF.icon = val; }
      else if(loc == lang){ DF.icon = val; }
    }
    else if(var=="TryExec"){ DF.tryexec = val; }
    else if(var=="Exec"){ DF.exec = val; }
    else if(var=="NoDisplay" && !DF.isHidden){ DF.isHidden = (val.toLower()=="true"); }
    else if(var=="Hidden" && !DF.isHidden){ DF.isHidden = (val.toLower()=="true"); }
    else if(var=="Categories"){ DF.catList = val.split(";",QString::SkipEmptyParts); }
    else if(var=="OnlyShowIn"){ DF.showInList = val.split(";",QString::SkipEmptyParts); }
    else if(var=="NotShowIn"){ DF.notShowInList = val.split(";",QString::SkipEmptyParts); }
    else if(var=="Terminal"){ DF.useTerminal= (val.toLower()=="true"); }
    else if(var=="Actions"){ DF.actionList = val.split(";",QString::SkipEmptyParts); }
    else if(var=="MimeType"){ DF.mimeList = val.split(";",QString::SkipEmptyParts); }
    else if(var=="Keywords"){ 
      if(DF.keyList.isEmpty() && loc.isEmpty()){ DF.keyList = val.split(";",QString::SkipEmptyParts); }
      else if(loc == lang){ DF.keyList = val.split(";",QString::SkipEmptyParts); }
    }
    else if(var=="StartupNotify"){ DF.startupNotify = (val.toLower()=="true"); }
    else if(var=="StartupWMClass"){ DF.startupWM = val; }
    else if(var=="URL"){ DF.url = val;}
    else if(var=="Type"){
      if(val.toLower()=="application"){ DF.type = XDGDesktop::APP; }
      else if(val.toLower()=="link"){ DF.type = XDGDesktop::LINK; }
      else if(val.toLower()=="dir"){ DF.type = XDGDesktop::DIR; }
      else{ DF.type = XDGDesktop::BAD; }
    }
  } //end reading file
  file.close();
  //If there are OnlyShowIn desktops listed, add them to the name
  if(!DF.showInList.isEmpty()){
    DF.name.append(" ("+DF.showInList.join(", ")+")");
  }
  //Return the structure
  ok=true;
  return DF;
}

bool LXDG::checkValidity(XDGDesktop dFile, bool showAll){
  bool ok=true;
  bool DEBUG = false;
  if(DEBUG){ qDebug() << "[LXDG] Check File validity:" << dFile.name << dFile.filePath; }
  switch (dFile.type){
    case XDGDesktop::BAD:
      ok=false; 
      if(DEBUG){ qDebug() << " - Bad file type"; }
      break;
    case XDGDesktop::APP:
      if(!dFile.tryexec.isEmpty() && !LXDG::checkExec(dFile.tryexec)){ ok=false; if(DEBUG){ qDebug() << " - tryexec does not exist";} }
      else if(dFile.exec.isEmpty() || dFile.name.isEmpty()){ ok=false; if(DEBUG){ qDebug() << " - exec or name is empty";} }
      else if(!LXDG::checkExec(dFile.exec.section(" ",0,0,QString::SectionSkipEmpty)) ){ ok=false; if(DEBUG){ qDebug() << " - first exec binary does not exist";} }
      break;
    case XDGDesktop::LINK:
      ok = !dFile.url.isEmpty();
      if(DEBUG && !ok){ qDebug() << " - Link with missing URL"; }
      break;
    case XDGDesktop::DIR:
      ok = !dFile.path.isEmpty();
      if(DEBUG && !ok){ qDebug() << " - Dir with missing path"; }
      break;
    default:
      ok=false;
      if(DEBUG){ qDebug() << " - Unknown file type"; } 
  }
  if(!showAll){
    if(!dFile.showInList.isEmpty()){ ok = dFile.showInList.contains("Lumina"); }
    else if(!dFile.notShowInList.isEmpty()){ ok = !dFile.notShowInList.contains("Lumina"); }
  }
  return ok;
}

bool LXDG::checkExec(QString exec){
  //Return true(good) or false(bad)
  if(exec.startsWith("/")){ return QFile::exists(exec); }
  else{
    QStringList paths = QString(getenv("PATH")).split(":");
    for(int i=0; i<paths.length(); i++){
      if(QFile::exists(paths[i]+"/"+exec)){ return true; }	    
    }
  }
  return false; //could not find the executable in the current path(s)
}

QStringList LXDG::systemApplicationDirs(){
  //Returns a list of all the directories where *.desktop files can be found
  QStringList appDirs = QString(getenv("XDG_DATA_HOME")).split(":");
  appDirs << QString(getenv("XDG_DATA_DIRS")).split(":");
  if(appDirs.isEmpty()){ appDirs << "/usr/local/share" << "/usr/share"; }
  //Now create a valid list
  QStringList out;
  for(int i=0; i<appDirs.length(); i++){
    if( QFile::exists(appDirs[i]+"/applications") ){
      out << appDirs[i]+"/applications";
      //Also check any subdirs within this directory 
      // (looking at you KDE - stick to the standards!!)
      QDir dir(appDirs[i]+"/applications");
      QStringList subs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
      //qDebug() << "Adding subdirectories:" << appDirs[i]+"/applications/["+subs.join(", ")+"]";
      for(int s=0; s<subs.length(); s++){ out << dir.absoluteFilePath(subs[s]); }
    }
  }
  return out;
}

QList<XDGDesktop> LXDG::systemDesktopFiles(bool showAll, bool showHidden){
  //Returns a list of all the unique *.desktop files that were found
  QStringList appDirs = LXDG::systemApplicationDirs();
  QStringList found; //for avoiding duplicate apps
  QList<XDGDesktop> out;
  bool ok; //for internal use only
  for(int i=0; i<appDirs.length(); i++){
      QDir dir(appDirs[i]);
      QStringList apps = dir.entryList(QStringList() << "*.desktop",QDir::Files, QDir::Name);
      for(int a=0; a<apps.length(); a++){
      	ok=false;
      	XDGDesktop dFile = LXDG::loadDesktopFile(dir.absoluteFilePath(apps[a]),ok);
      	if( LXDG::checkValidity(dFile, showAll) ){
      	  if( !found.contains(dFile.name) && (!dFile.isHidden || showHidden) ){
      	    out << dFile;
      	    found << dFile.name;
      	  }
      	}
      }
  }
  return out;
}

QHash<QString,QList<XDGDesktop> > LXDG::sortDesktopCats(QList<XDGDesktop> apps){
  //Sort the list of applications into their different categories (main categories only)
  //Create the category lists
  QList<XDGDesktop> multimedia, dev, ed, game, graphics, network, office, science, settings, sys, utility, other;
  //Sort the apps into the lists
  for(int i=0; i<apps.length(); i++){
    if(apps[i].catList.contains("AudioVideo")){ multimedia << apps[i]; }
    else if(apps[i].catList.contains("Development")){ dev << apps[i]; }
    else if(apps[i].catList.contains("Education")){ ed << apps[i]; }
    else if(apps[i].catList.contains("Game")){ game << apps[i]; }
    else if(apps[i].catList.contains("Graphics")){ graphics << apps[i]; }
    else if(apps[i].catList.contains("Network")){ network << apps[i]; }
    else if(apps[i].catList.contains("Office")){ office << apps[i]; }
    else if(apps[i].catList.contains("Science")){ science << apps[i]; }
    else if(apps[i].catList.contains("Settings")){ settings << apps[i]; }
    else if(apps[i].catList.contains("System")){ sys << apps[i]; }
    else if(apps[i].catList.contains("Utility")){ utility << apps[i]; }
    else{ other << apps[i]; }
  }
  //Now create the output hash
  QHash<QString,QList<XDGDesktop> > out;
  if(!multimedia.isEmpty()){ out.insert("Multimedia", LXDG::sortDesktopNames(multimedia)); }
  if(!dev.isEmpty()){ out.insert("Development", LXDG::sortDesktopNames(dev)); }
  if(!ed.isEmpty()){ out.insert("Education", LXDG::sortDesktopNames(ed)); }
  if(!game.isEmpty()){ out.insert("Game", LXDG::sortDesktopNames(game)); }
  if(!graphics.isEmpty()){ out.insert("Graphics", LXDG::sortDesktopNames(graphics)); }
  if(!network.isEmpty()){ out.insert("Network", LXDG::sortDesktopNames(network)); }
  if(!office.isEmpty()){ out.insert("Office", LXDG::sortDesktopNames(office)); }
  if(!science.isEmpty()){ out.insert("Science", LXDG::sortDesktopNames(science)); }
  if(!settings.isEmpty()){ out.insert("Settings", LXDG::sortDesktopNames(settings)); }
  if(!sys.isEmpty()){ out.insert("System", LXDG::sortDesktopNames(sys)); }
  if(!utility.isEmpty()){ out.insert("Utility", LXDG::sortDesktopNames(utility)); }
  if(!other.isEmpty()){ out.insert("Unsorted", LXDG::sortDesktopNames(other)); }
  //return the resulting hash
  return out;
}

QList<XDGDesktop> LXDG::sortDesktopNames(QList<XDGDesktop> apps){
  //Sort the list by name of the application
  QHash<QString, XDGDesktop> sorter;
  for(int i=0; i<apps.length(); i++){
    sorter.insert(apps[i].name, apps[i]);	  
  }
  QStringList keys = sorter.keys();
  keys.sort();
  //Re-assemble the output list
  QList<XDGDesktop> out;
  for(int i=0; i<keys.length(); i++){
    out << sorter[keys[i]];
  }
  return out;
}

QString LXDG::getDesktopExec(XDGDesktop app){
  //Generate the executable line for the application
  QString out;
  if(app.exec.isEmpty()){ return ""; }
  else if(app.useTerminal){
    out = "xterm -lc -e "+app.exec;  
  }else{
    out = app.exec;	  
  }
  return out;
}

void LXDG::setEnvironmentVars(){
  //Set the default XDG environment variables if not already set
  setenv("XDG_DATA_HOME",QString(QDir::homePath()+"/.local/share").toUtf8(), 0);
  setenv("XDG_CONFIG_HOME",QString(QDir::homePath()+"/.config").toUtf8(), 0);
  setenv("XDG_DATA_DIRS","/usr/local/share:/usr/share", 0);
  setenv("XDG_CONFIG_DIRS","/etc/xdg", 0);
  setenv("XDG_CACHE_HOME",QString(QDir::homePath()+"/.cache").toUtf8(), 0);
  //Don't set "XDG_RUNTIME_DIR" yet - need to look into the specs
}

QIcon LXDG::findIcon(QString iconName, QString fallback){
  //Check if the icon is an absolute path and exists
  if(QFile::exists(iconName) && iconName.startsWith("/")){ return QIcon(iconName); }
  else if(iconName.startsWith("/")){ iconName.section("/",-1); } //Invalid absolute path, just looks for the icon
  //Check if the icon is actually given
  if(iconName.isEmpty()){ return QIcon(fallback); }
  //Now try to find the icon from the theme
  bool DEBUG = false;
  if(DEBUG){ qDebug() << "[LXDG] Find icon for:" << iconName; }
  //Check the default theme search paths
  QStringList paths = QIcon::themeSearchPaths();
  if(paths.isEmpty()){
    //Set the XDG default icon theme search paths
    paths << QDir::homePath()+"/.icons";
    QStringList xdd = QString(getenv("XDG_DATA_HOME")).split(":");
    xdd << QString(getenv("XDG_DATA_DIRS")).split(":");
    for(int i=0; i<xdd.length(); i++){
      paths << xdd[i]+"/icons";	    
    }
    #ifdef __FreeBSD__
    paths << "/usr/local/share/pixmaps";
    #endif
    #ifdef __linux__
    paths << "/usr/share/pixmaps";
    #endif
    QIcon::setThemeSearchPaths(paths);
  }
  if(DEBUG){ qDebug() << "[LXDG] Icon search paths:" << paths; }
  //Finding an icon from the current theme is already built into Qt, just use it
  QString cTheme = QIcon::themeName();
  if(cTheme.isEmpty()){ QIcon::setThemeName("oxygen"); } //set the XDG default theme
  if(DEBUG){ qDebug() << "[LXDG] Current theme:" << cTheme; }
  //Try to load the icon from the current theme
  QIcon ico = QIcon::fromTheme(iconName);
  //Try to load the icon from /usr/local/share/pixmaps
  if( ico.isNull() ){
    //qDebug() << "Could not find icon:" << iconName;
    #ifdef __FreeBSD__
    QDir base("/usr/local/share/pixmaps");
    #endif
    #ifdef __linux__
    QDir base("/usr/share/pixmaps");
    #endif
    QStringList matches = base.entryList(QStringList() << "*"+iconName+"*", QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    if( !matches.isEmpty() ){
      #ifdef __FreeBSD__
      ico = QIcon("/usr/local/share/pixmaps/"+matches[0]); //just use the first match
      #endif
      #ifdef __linux__
      ico = QIcon("/usr/share/pixmaps/"+matches[0]); //just use the first match
      #endif
    }else{
      //Fallback on a manual search over the default theme directories (hicolor, then oxygen)
      if( QDir::searchPaths("fallbackicons").isEmpty() ){
        //Set the fallback search paths
        #ifdef __FreeBSD__
        QString base = "/usr/local/share/icons/";
        #endif
        #ifdef __linux__
        QString base = "/usr/share/icons/";
        #endif
        QDir::setSearchPaths("fallbackicons", QStringList() << getChildIconDirs(base+"hicolor") << getChildIconDirs(base+"oxygen") ); 
      }
      if(QFile::exists("fallbackicons:"+iconName+".png")){
        ico = QIcon("fallbackicons:"+iconName+".png");
      }
    }
  }
  //Use the fallback icon if necessary
  if(ico.isNull() && !fallback.isEmpty()){
    ico = LXDG::findIcon(fallback,"");	  
  }
  //Return the icon
  return ico;
}

QStringList LXDG::getChildIconDirs(QString parent){
  //This is a recursive function that returns the absolute path(s) of directories with *.png files
  QDir D(parent);
  QStringList out;
  QStringList dirs = D.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
  QStringList pngs = D.entryList(QStringList() << "*.png", QDir::Files | QDir::NoDotAndDotDot, QDir::NoSort);
  if(pngs.length() > 0){ out << D.absolutePath(); }
  for(int i=0; i<dirs.length(); i++){
    pngs.clear();
    pngs = getChildIconDirs(D.absoluteFilePath(dirs[i])); //re-use the old list variable
    if(pngs.length() > 0){ out << pngs; }
  }
  return out;
}

QStringList LXDG::systemMimeDirs(){
  //Returns a list of all the directories where *.xml MIME files can be found
  QStringList appDirs = QString(getenv("XDG_DATA_HOME")).split(":");
  appDirs << QString(getenv("XDG_DATA_DIRS")).split(":");
  if(appDirs.isEmpty()){ appDirs << "/usr/local/share" << "/usr/share"; }
  //Now create a valid list
  QStringList out;
  for(int i=0; i<appDirs.length(); i++){
    if( QFile::exists(appDirs[i]+"/mime") ){
      out << appDirs[i]+"/mime";	    
    }
  }
  return out;
}

QIcon LXDG::findMimeIcon(QString extension){
  QIcon ico;
  QString mime = LXDG::findAppMimeForFile(extension);
  if(mime.isEmpty()){ mime = LXDG::findAppMimeForFile(extension.toLower()); }
  mime.replace("/","-"); //translate to icon mime name
  if(!mime.isEmpty()){ ico = LXDG::findIcon(mime, "unknown");} //use the "unknown" mimetype icon as fallback	
  if(ico.isNull()){ ico = LXDG::findIcon("unknown",""); } //just in case
  return ico;
}

QString LXDG::findAppMimeForFile(QString extension){
  QString out;
  int weight = 0;
  QStringList mimes = LXDG::loadMimeFileGlobs2().filter(":*."+extension);
  for(int m=0; m<mimes.length(); m++){
    QString mime = mimes[m].section(":",1,1,QString::SectionSkipEmpty);
    if(mimes[m].section(":",0,0,QString::SectionSkipEmpty).toInt() > weight ){
      out = mime;
    }
  }
  return out;
}

QStringList LXDG::findFilesForMime(QString mime){
  QStringList out;
  QStringList mimes = LXDG::loadMimeFileGlobs2().filter(mime);
  for(int i=0; i<mimes.length(); i++){
    out << mimes[i].section(":",2,2); // "*.<extension>"
  }
  //qDebug() << "Mime to Files:" << mime << out;
  return out;
}

QStringList LXDG::loadMimeFileGlobs2(){
  //output format: <weight>:<mime type>:<file extension (*.something)>
  if(mimeglobs.isEmpty() || (mimechecktime < (QDateTime::currentMSecsSinceEpoch()-30000)) ){
    //qDebug() << "Loading globs2 mime DB files";
    mimeglobs.clear();
    mimechecktime = QDateTime::currentMSecsSinceEpoch(); //save the current time this was last checked
    QStringList dirs = LXDG::systemMimeDirs();
    for(int i=0; i<dirs.length(); i++){
      if(QFile::exists(dirs[i]+"/globs2")){
        QFile file(dirs[i]+"/globs2");
        if(!file.exists()){ continue; }
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){ continue; }
        QTextStream in(&file);
        while(!in.atEnd()){
          QString line = in.readLine();
          if(!line.startsWith("#")){
            mimeglobs << line.simplified();
          }
        }
	file.close();
      }
    }    
  }
  return mimeglobs;
}

