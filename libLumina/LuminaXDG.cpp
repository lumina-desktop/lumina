//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LuminaXDG.h"
#include "LuminaOS.h"
#include <QObject>
#include <QMediaPlayer>

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
    DF.type = XDGDesktop::APP;
    DF.filePath = filePath;
    DF.exec = DF.tryexec = "";   // just to make sure this is initialized
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
  bool insection=false;
  while(!os.atEnd()){
    QString line = os.readLine();
    //Check that this is the entry portion of the file (not the action/other sections)
    if(line=="[Desktop Entry]"){ insection=true; continue; }
    else if(line.startsWith("[")){ insection=false; }
    if(!insection || line.startsWith("#")){ continue; }
    //Now parse out the file
    line = line.simplified();
    QString var = line.section("=",0,0).simplified();
    QString loc = var.section("[",1,1).section("]",0,0).simplified(); // localization
    var = var.section("[",0,0).simplified(); //remove the localization
    QString val = line.section("=",1,50).simplified();
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
    else if( (var=="TryExec") && (DF.tryexec.isEmpty()) ) { DF.tryexec = val; }
    else if( (var=="Exec") && (DF.exec.isEmpty() ) ) { DF.exec = val; }   // only take the first Exec command in the file
    else if( (var=="Path") && (DF.path.isEmpty() ) ){ DF.path = val; }
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
      else{ DF.type = XDGDesktop::BAD; } //Unknown type
    }
  } //end reading file
  file.close();
  //If there are OnlyShowIn desktops listed, add them to the name
  if(!DF.showInList.isEmpty()){
    DF.name.append(" ("+DF.showInList.join(", ")+")");
  }
  //Quick fix for showing "wine" applications (which quite often don't list a category)
  if(DF.catList.isEmpty() && filePath.contains("/wine/")){
    DF.catList << "Wine"; //Internal Lumina category only (not in XDG specs as of 11/14/14)
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
      out << LUtils::listSubDirectories(appDirs[i]+"/applications");
      //QDir dir(appDirs[i]+"/applications");
      //QStringList subs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
      //qDebug() << "Adding subdirectories:" << appDirs[i]+"/applications/["+subs.join(", ")+"]";
      //for(int s=0; s<subs.length(); s++){ out << dir.absoluteFilePath(subs[s]); }
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
  QList<XDGDesktop> multimedia, dev, ed, game, graphics, network, office, science, settings, sys, utility, other, wine;
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
    else if(apps[i].catList.contains("Wine")){ wine << apps[i]; }
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
  if(!wine.isEmpty()){ out.insert("Wine", LXDG::sortDesktopNames(wine)); }
  if(!other.isEmpty()){ out.insert("Unsorted", LXDG::sortDesktopNames(other)); }
  //return the resulting hash
  return out;
}

QList<XDGDesktop> LXDG::sortDesktopNames(QList<XDGDesktop> apps){
  //Sort the list by name of the application
  QHash<QString, XDGDesktop> sorter;
  for(int i=0; i<apps.length(); i++){
    sorter.insert(apps[i].name.toLower(), apps[i]);	  
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
  //Now perform any of the XDG flag substitutions as appropriate (9/2014 standards)
  if(out.contains("%i") && !app.icon.isEmpty() ){ out.replace("%i", "--icon \'"+app.icon+"\'"); }
  if(out.contains("%c")){ 
    if(!app.name.isEmpty()){ out.replace("%c", "\'"+app.name+"\'"); }
    else if(!app.genericName.isEmpty()){ out.replace("%c", "\'"+app.genericName+"\'"); }
    else{ out.replace("%c", "\'"+app.filePath.section("/",-1).section(".desktop",0,0)+"\'"); }
  }
  if(out.contains("%k")){ out.replace("%k", "\'"+app.filePath+"\'"); }
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
  bool DEBUG =false;
  if(DEBUG){ qDebug() << "[LXDG] Find icon for:" << iconName; }
  if(QFile::exists(iconName) && iconName.startsWith("/")){ return QIcon(iconName); }
  else if(iconName.startsWith("/")){ iconName.section("/",-1); } //Invalid absolute path, just looks for the icon
  //Check if the icon is actually given
  if(iconName.isEmpty()){ 
    if(fallback.isEmpty()){  return QIcon(); }
    else{ return LXDG::findIcon(fallback, ""); }
  }
  //Now try to find the icon from the theme
  if(DEBUG){ qDebug() << "[LXDG] Start search for icon"; }
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
    paths << LOS::AppPrefix()+"share/pixmaps";
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
    QDir base(LOS::AppPrefix()+"share/pixmaps");
    QStringList matches = base.entryList(QStringList() << "*"+iconName+"*", QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    if( !matches.isEmpty() ){
      ico = QIcon(base.absoluteFilePath(matches[0])); //just use the first match
    }else{
      //Fallback on a manual search over the default theme directories (hicolor, then oxygen)
      if( QDir::searchPaths("fallbackicons").isEmpty() ){
        //Set the fallback search paths
	QString localbase = LOS::AppPrefix()+"share/icons/";
        QDir::setSearchPaths("fallbackicons", QStringList() << getChildIconDirs(localbase+"hicolor") << getChildIconDirs(localbase+"oxygen") ); 
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

QString LXDG::findAppMimeForFile(QString filename, bool multiple){
  QString out;
  QString extension = filename.section(".",-1);
  if("."+extension == filename){ extension.clear(); } //hidden file without extension
  //qDebug() << "MIME SEARCH:" << filename << extension;
  QStringList mimefull = LXDG::loadMimeFileGlobs2();
  QStringList mimes;
  //Just in case the extension/filename is a mimetype itself
  if( mimefull.filter(":"+filename+":").length() == 1){
    return filename;
  }
  else if(mimefull.filter(":"+extension+":").length() == 1){
    return extension;
  }
  //Look for globs at the end of the filename
  if(!extension.isEmpty()){ 
    mimes = mimefull.filter(":*."+extension); 
    //If nothing found, try a case-insensitive search
    if(mimes.isEmpty()){ mimes = mimefull.filter(":*."+extension, Qt::CaseInsensitive); }
    //Now ensure that the filter was accurate (*.<extention>.<something> will still be caught)
    for(int i=0; i<mimes.length(); i++){
      if(!filename.endsWith( mimes[i].section(":*",-1), Qt::CaseInsensitive )){ mimes.removeAt(i); i--; }
    }
  }
  //Look for globs at the start of the filename
  if(mimes.isEmpty()){ 
    mimes = mimefull.filter(":"+filename.left(2)); //look for the first 2 characters initially
	//Note: This initial filter will only work if the wildcard (*) is not within the first 2 characters of the pattern
    //Now ensure that the filter was accurate
    for(int i=0; i<mimes.length(); i++){
      if(!filename.startsWith( mimes[i].section(":",3,50,QString::SectionSkipEmpty).section("*",0,0), Qt::CaseInsensitive )){ mimes.removeAt(i); i--; }
    }
  } 
  mimes.sort(); //this automatically puts them in weight order (100 on down)
  QStringList matches;
  //qDebug() << "Mimes:" << mimes;
  for(int m=0; m<mimes.length(); m++){
    QString mime = mimes[m].section(":",1,1,QString::SectionSkipEmpty);
    matches << mime;
  }
  //qDebug() << "Matches:" << matches;
  if(multiple){ out = matches.join("::::"); }
  else if( !matches.isEmpty() ){ out = matches.first(); }
  else{ out.clear(); }
  //qDebug() << "Out:" << out;
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

QStringList LXDG::listFileMimeDefaults(){
  //This will spit out a itemized list of all the mimetypes and relevant info
  // Output format: <mimetype>::::<extension>::::<default>::::<localized comment>
  QStringList mimes = LXDG::loadMimeFileGlobs2();
  //Remove all the application files from the list (only a single app defines/uses this type in general)
  /*QStringList apps = mimes.filter(":application/");
  //qDebug() << "List Mime Defaults";
  for(int i=0; i<apps.length(); i++){ mimes.removeAll(apps[i]); }*/
  //Now start filling the output list
  QStringList out;
  for(int i=0; i<mimes.length(); i++){
    QString mimetype = mimes[i].section(":",1,1);
    QStringList tmp = mimes.filter(mimetype);
    //Collect all the different extensions with this mimetype
    QStringList extlist;
    for(int j=0; j<tmp.length(); j++){
      mimes.removeAll(tmp[j]);
      extlist << tmp[j].section(":",2,2);
    }
    extlist.removeDuplicates(); //just in case
    //Now look for a current default for this mimetype
    QString dapp = LXDG::findDefaultAppForMime(mimetype); //default app;
    
    //Create the output entry
    //qDebug() << "Mime entry:" << i << mimetype << dapp;
    out << mimetype+"::::"+extlist.join(", ")+"::::"+dapp+"::::"+LXDG::findMimeComment(mimetype);
    
    i--; //go back one (continue until the list is empty)
  }
  return out;
}

QString LXDG::findMimeComment(QString mime){
  QString comment;
  QStringList dirs = LXDG::systemMimeDirs();
  QString lang = QString(getenv("LANG")).section(".",0,0);
  QString shortlang = lang.section("_",0,0);
  for(int i=0; i<dirs.length(); i++){
    if(QFile::exists(dirs[i]+"/"+mime+".xml")){
      QStringList info = LUtils::readFile(dirs[i]+"/"+mime+".xml");
      QStringList filter = info.filter("<comment xml:lang=\""+lang+"\">");
      //First look for a full language match, then short language, then general comment
      if(filter.isEmpty()){ filter = info.filter("<comment xml:lang=\""+shortlang+"\">"); }
      if(filter.isEmpty()){ filter = info.filter("<comment>"); }
      if(!filter.isEmpty()){
        comment = filter.first().section(">",1,1).section("</",0,0);
        break;
      }
    }
  }
  return comment;
}

QString LXDG::findDefaultAppForMime(QString mime){
  //First get the priority-ordered list of default file locations
  QStringList dirs;
  dirs << QString(getenv("XDG_CONFIG_HOME"))+"/lumina-mimeapps.list" \
	 << QString(getenv("XDG_CONFIG_HOME"))+"/mimeapps.list";
  QStringList tmp = QString(getenv("XDG_CONFIG_DIRS")).split(":");
	for(int i=0; i<tmp.length(); i++){ dirs << tmp[i]+"/lumina-mimeapps.list"; }
	for(int i=0; i<tmp.length(); i++){ dirs << tmp[i]+"/mimeapps.list"; }
  dirs << QString(getenv("XDG_DATA_HOME"))+"/applications/lumina-mimeapps.list" \
	 << QString(getenv("XDG_DATA_HOME"))+"/applications/mimeapps.list";  
  tmp = QString(getenv("XDG_DATA_DIRS")).split(":");
	for(int i=0; i<tmp.length(); i++){ dirs << tmp[i]+"/applications/lumina-mimeapps.list"; }
	for(int i=0; i<tmp.length(); i++){ dirs << tmp[i]+"/applications/mimeapps.list"; }
	
  //Now go through all the files in order of priority until a default is found
  QString cdefault;
  QStringList white; //lists to keep track of during the search (black unused at the moment)
  for(int i=0; i<dirs.length() && cdefault.isEmpty(); i++){
    if(!QFile::exists(dirs[i])){ continue; }
    QStringList info = LUtils::readFile(dirs[i]);
    if(info.isEmpty()){ continue; }
    QString workdir = dirs[i].section("/",0,-1); //just the directory
    int def = info.indexOf("[Default Applications]"); //find this line to start on
    if(def>=0){
      for(int d=def+1; d<info.length(); d++){
        if(info[d].startsWith("[")){ break; } //starting a new section now - finished with defaults
	if(info[d].contains(mime+"=")){
	  white << info[d].section("=",1,50).split(";");
	  break;
	}
      }
    }
    // Now check for any white-listed files in this work dir 
    // find the full path to the file (should run even if nothing in this file)
    for(int w=0; w<white.length(); w++){
      //First check for absolute paths to *.desktop file
      if( white[w].startsWith("/") ){
	 if( QFile::exists(white[w]) ){ cdefault=white[w]; break; }
	 else{ white.removeAt(w); w--; } //invalid file path - remove it from the list
      }
      //Now check for relative paths to  file (in current priority-ordered work dir)
      else if( QFile::exists(workdir+"/"+white[w]) ){ cdefault=workdir+"/"+white[w]; break; }
    }   
    /* WRITTEN BUT UNUSED CODE FOR MIMETYPE ASSOCIATIONS
    //Skip using this because it is simply an alternate/unsupported standard that conflicts with
      the current mimetype database standards. It is better/faster to parse 1 or 2 database glob files
      rather than have to iterate through hundreds of *.desktop files *every* time you need to
      find an application
    
    if(addI<0 && remI<0){
      //Simple Format: <mimetype>=<*.desktop file>;<*.desktop file>;.....
        // (usually only one desktop file listed)
      info = info.filter(mimetype+"=");
      //Load the listed default(s)
      for(int w=0; w<info.length(); w++){
        white << info[w].section("=",1,50).split(";");
      }
    }else{
      //Non-desktop specific mimetypes file: has a *very* convoluted/inefficient algorithm (required by spec)
      if(addI<0){ addI = info.length(); } //no add section
      if(remI<0){ remI = info.length(); } // no remove section:
      //Whitelist items
      for(int a=addI+1; a!=remI && a<info.length(); a++){
        if(info[a].contains(mimetype+"=")){ 
	  QStringList tmp = info[a].section("=",1,50).split(";"); 
	  for(int t=0; t<tmp.length(); t++){ 
	    if(!black.contains(tmp[t])){ white << tmp[t]; } //make sure this item is not on the black list
	  }
	  break;
	}
      }
      //Blacklist items
      for(int a=remI+1; a!=addI && a<info.length(); a++){
        if(info[a].contains(mimetype+"=")){ black << info[a].section("=",1,50).split(";"); break;}
      }
      
      //STEPS 3/4 not written yet
      
    } //End of non-DE mimetypes file */
    
  } //End loop over files
	
  return cdefault;
}

QStringList LXDG::findAvailableAppsForMime(QString mime){
  QStringList dirs = LXDG::systemApplicationDirs();
  QStringList out;
  //Loop over all possible directories that contain *.destop files
  //  and check for the mimeinfo.cache file
  for(int i=0; i<dirs.length(); i++){
    if(QFile::exists(dirs[i]+"/mimeinfo.cache")){
      QStringList matches = LUtils::readFile(dirs[i]+"/mimeinfo.cache").filter(mime+"=");
      //Find any matches for our mimetype in the cache
      for(int j=0; j<matches.length(); j++){
        QStringList files = matches[j].section("=",1,1).split(";",QString::SkipEmptyParts);
	//Verify that each file exists before putting the full path to the file in the output
	for(int m=0; m<files.length(); m++){
	  if(QFile::exists(dirs[i]+"/"+files[m])){
	    out << dirs[i]+"/"+files[m];
	  }else if(files[m].contains("-")){ //kde4-<filename> -> kde4/<filename> (stupid KDE variations!!)
	    files[m].replace("-","/");
	    if(QFile::exists(dirs[i]+"/"+files[m])){
	      out << dirs[i]+"/"+files[m];
	    }
	  }
	}
      }
    }
  }
  //qDebug() << "Found Apps for Mime:" << mime << out << dirs;
  return out;
}

void LXDG::setDefaultAppForMime(QString mime, QString app){
  QString filepath = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-mimeapps.list";
  QStringList cinfo = LUtils::readFile(filepath);
  //If this is a new file, make sure to add the header appropriately
  if(cinfo.isEmpty()){ cinfo << "#Automatically generated with lumina-config" << "# DO NOT CHANGE MANUALLY" << "[Default Applications]"; }
  //Check for any current entry for this mime type
  QStringList tmp = cinfo.filter(mime+"=");
  int index = -1;
  if(!tmp.isEmpty()){ index = cinfo.indexOf(tmp.first()); }
  //Now add the new default entry (if necessary)
  if(app.isEmpty()){
    if(index>=0){ cinfo.removeAt(index); } //Remove entry
  }else{
    if(index<0){
      cinfo << mime+"="+app+";"; //new entry
    }else{
      cinfo[index] = mime+"="+app+";"; //overwrite existing entry
    }
  }
  LUtils::writeFile(filepath, cinfo, true);
  return;
}

QStringList LXDG::findAVFileExtensions(){
  //output format: QDir name filter for valid A/V file extensions
  QStringList globs = LXDG::loadMimeFileGlobs2();
  QStringList av = globs.filter(":audio/");
  av << globs.filter(":video/");
  for(int i=0; i<av.length(); i++){
    //Just use all audio/video mimetypes (for now)
    av[i] = av[i].section(":",2,2);
    //Qt5 Auto detection (broken - QMediaPlayer seg faults with Qt 5.3 - 11/24/14)
    /*if( QMultimedia::NotSupported != QMediaPlayer::hasSupport(av[i].section(":",1,1)) ){ av[i] = av[i].section(":",2,2); }
    else{ av.removeAt(i); i--; }*/
  }
  av.removeDuplicates();
  return av;
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

