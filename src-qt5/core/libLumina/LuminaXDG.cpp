//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LuminaXDG.h"
#include "LuminaOS.h"
#include "LuminaUtils.h"
#include <QObject>
#include <QMediaPlayer>
#include <QSvgRenderer>

static QStringList mimeglobs;
static qint64 mimechecktime;

//==== LFileInfo Functions ====
//Need some extra information not usually available by a QFileInfo
void LFileInfo::loadExtraInfo(){
  //Now load the extra information
  if(this->isDir()){
    mime = "inode/directory";
    //Special directory icons
    QString name = this->fileName().toLower();
    if(name=="desktop"){ icon = "user-desktop"; }
    else if(name=="tmp"){ icon = "folder-temp"; }
    else if(name=="video" || name=="videos"){ icon = "folder-video"; }
    else if(name=="music" || name=="audio"){ icon = "folder-sound"; }
    else if(name=="projects" || name=="devel"){ icon = "folder-development"; }
    else if(name=="notes"){ icon = "folder-txt"; }
    else if(name=="downloads"){ icon = "folder-downloads"; }
    else if(name=="documents"){ icon = "folder-documents"; }
    else if(name=="images" || name=="pictures"){ icon = "folder-image"; }
    else if( !this->isReadable() ){ icon = "folder-locked"; }
  }else if( this->suffix()=="desktop"){
    mime = "application/x-desktop";
    icon = "application-x-desktop"; //default value
    bool ok = false;
    desk = LXDG::loadDesktopFile(this->absoluteFilePath(), ok);
    if(ok){
      //use the specific desktop file info (if possible)
      if(!desk.icon.isEmpty()){ icon = desk.icon; }
    }
  }else{
    //Generic file, just determine the mimetype
    mime = LXDG::findAppMimeForFile(this->fileName());
  }
}
LFileInfo::LFileInfo(QString filepath){ //overloaded contructor
  this->setFile(filepath);
  loadExtraInfo();
}	
LFileInfo::LFileInfo(QFileInfo info){ //overloaded contructor
  this->swap(info); //use the given QFileInfo without re-loading it
  loadExtraInfo();
}		

//Functions for accessing the extra information
// -- Return the mimetype for the file
QString LFileInfo::mimetype(){
  if(mime=="inode/directory"){ return ""; }
  else{ return mime; }
}

// -- Return the icon to use for this file
QString LFileInfo::iconfile(){
  if(!icon.isEmpty()){
    return icon;
  }else{
    if(!mime.isEmpty()){
      QString tmp = mime; 
      tmp.replace("/","-");
      return tmp;
    }else if(this->isExecutable()){
      return "application-x-executable";
    }
  }
  return ""; //Fall back to nothing
}

// -- Check if this is an XDG desktop file
bool LFileInfo::isDesktopFile(){
  return (!desk.filePath.isEmpty());	
}

// -- Allow access to the XDG desktop data structure
XDGDesktop* LFileInfo::XDG(){
  return &desk;
}

// -- Check if this is a readable image file (for thumbnail support)
bool LFileInfo::isImage(){
  if(!mime.startsWith("image/")){ return false; } //quick return for non-image files
  //Check the Qt subsystems to see if this image file can be read
  return ( !LUtils::imageExtensions().filter(this->suffix().toLower()).isEmpty() );
}

bool LFileInfo::isAVFile(){
  return (mime.startsWith("audio/") || mime.startsWith("video/") );
}


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
    DF.lastRead = QDateTime::currentDateTime();
    DF.exec = DF.tryexec = "";   // just to make sure this is initialized

  //Get the current localization code
  QString lang = QLocale::system().name(); //lang code
  QString slang = lang.section("_",0,0); //short lang code

  //Read in the File
  bool insection=false;
  bool inaction=false;
  QStringList file = LUtils::readFile(filePath);
  if(file.isEmpty()){ return DF; }
  //if(filePath.contains("pcbsd")){ qDebug() << "Check File:" << filePath << lang << slang; }
  XDGDesktopAction CDA; //current desktop action
  for(int i=0; i<file.length(); i++){
    QString line = file[i];
    //if(filePath.contains("pcbsd")){ qDebug() << " - Check Line:" << line << inaction << insection; }
    //Check if this is the end of a section
    if(line.startsWith("[") && inaction){ 
      insection=false; inaction=false;
      //Add the current Action structure to the main desktop structure if appropriate
      if(!CDA.ID.isEmpty()){ DF.actions << CDA; CDA = XDGDesktopAction(); }
    }else if(line.startsWith("[")){ insection=false; inaction = false; }
    //Now check if this is the beginning of a section
    if(line=="[Desktop Entry]"){ insection=true; continue; }
    else if(line.startsWith("[Desktop Action ")){ 
      //Grab the ID of the action out of the label
      CDA.ID = line.section("]",0,0).section("Desktop Action",1,1).simplified();
      inaction = true;
      continue;
    }else if( (!insection && !inaction) || line.startsWith("#")){ continue; }
    //Now parse out the file
    line = line.simplified();
    QString var = line.section("=",0,0).simplified();
    QString loc = var.section("[",1,1).section("]",0,0).simplified(); // localization
    var = var.section("[",0,0).simplified(); //remove the localization
    QString val = line.section("=",1,50).simplified();
    //if(filePath.contains("pcbsd")){ qDebug() << " -- " << var << val << loc; }
    //-------------------
    if(var=="Name"){ 
      if(insection){
        if(DF.name.isEmpty() && loc.isEmpty()){ DF.name = val; }
	else if(DF.name.isEmpty() && loc==slang){ DF.name = val; } //short locale code
        else if(loc == lang){ DF.name = val; }
      }else if(inaction){
        if(CDA.name.isEmpty() && loc.isEmpty()){ CDA.name = val; }
	else if(CDA.name.isEmpty() && loc==slang){ CDA.name = val; } //short locale code
        else if(loc == lang){ CDA.name = val; }	      
      }
      //hasName = true;
    }else if(var=="GenericName" && insection){ 
      if(DF.genericName.isEmpty() && loc.isEmpty()){ DF.genericName = val; }
      else if(DF.genericName.isEmpty() && loc==slang){ DF.genericName = val; } //short locale code
      else if(loc == lang){ DF.genericName = val; }
    }else if(var=="Comment" && insection){ 
      if(DF.comment.isEmpty() && loc.isEmpty()){ DF.comment = val; }
      else if(DF.comment.isEmpty() && loc==slang){ DF.comment = val; } //short locale code
      else if(loc == lang){ DF.comment = val; }
    }else if(var=="Icon"){ 
      if(insection){
        if(DF.icon.isEmpty() && loc.isEmpty()){ DF.icon = val; }
	else if(DF.icon.isEmpty() && loc==slang){ DF.icon = val; } //short locale code
        else if(loc == lang){ DF.icon = val; }
      }else if(inaction){
	if(CDA.icon.isEmpty() && loc.isEmpty()){ CDA.icon = val; }
	else if(CDA.icon.isEmpty() && loc==slang){ CDA.icon = val; } //short locale code
        else if(loc == lang){ CDA.icon = val; }
      }
    }
    else if( (var=="TryExec") && (DF.tryexec.isEmpty()) && insection) { DF.tryexec = val; }
    else if(var=="Exec"){
      if(insection && DF.exec.isEmpty() ){ DF.exec = val; }
      else if(inaction && CDA.exec.isEmpty() ){ CDA.exec = val; }
    }	    
    else if( (var=="Path") && (DF.path.isEmpty() ) && insection){ DF.path = val; }
    else if(var=="NoDisplay" && !DF.isHidden && insection){ DF.isHidden = (val.toLower()=="true"); }
    else if(var=="Hidden" && !DF.isHidden && insection){ DF.isHidden = (val.toLower()=="true"); }
    else if(var=="Categories" && insection){ DF.catList = val.split(";",QString::SkipEmptyParts); }
    else if(var=="OnlyShowIn" && insection){ DF.showInList = val.split(";",QString::SkipEmptyParts); }
    else if(var=="NotShowIn" && insection){ DF.notShowInList = val.split(";",QString::SkipEmptyParts); }
    else if(var=="Terminal" && insection){ DF.useTerminal= (val.toLower()=="true"); }
    else if(var=="Actions" && insection){ DF.actionList = val.split(";",QString::SkipEmptyParts); }
    else if(var=="MimeType" && insection){ DF.mimeList = val.split(";",QString::SkipEmptyParts); }
    else if(var=="Keywords" && insection){ 
      if(DF.keyList.isEmpty() && loc.isEmpty()){ DF.keyList = val.split(";",QString::SkipEmptyParts); }
      else if(loc == lang){ DF.keyList = val.split(";",QString::SkipEmptyParts); }
    }
    else if(var=="StartupNotify" && insection){ DF.startupNotify = (val.toLower()=="true"); }
    else if(var=="StartupWMClass" && insection){ DF.startupWM = val; }
    else if(var=="URL" && insection){ DF.url = val;}
    else if(var=="Type" && insection){
      if(val.toLower()=="application"){ DF.type = XDGDesktop::APP; }
      else if(val.toLower()=="link"){ DF.type = XDGDesktop::LINK; }
      else if(val.toLower()=="dir"){ DF.type = XDGDesktop::DIR; }
      else{ DF.type = XDGDesktop::BAD; } //Unknown type
      //hasType = true;
    }
  } //end reading file
  //file.close();
  //If there are OnlyShowIn desktops listed, add them to the name
  if( !DF.showInList.isEmpty() && !DF.showInList.contains("Lumina", Qt::CaseInsensitive) ){
    /*QStringList added;
    //Need to be careful about case insensitivity here - the QList functions don't understand it
    for(int i=0; i<DF.showInList.length(); i++){
      if(DF.showInList[i].toLower()!="lumina"){ added << DF.showInList[i]; }
    }*/
    //if(!added.isEmpty()){ 
      DF.name.append(" ("+DF.showInList.join(", ")+")"); 
    //}
  }
  //Quick fix for showing "wine" applications (which quite often don't list a category, or have other differences)
  if(DF.catList.isEmpty() && filePath.contains("/wine/")){
    DF.catList << "Wine"; //Internal Lumina category only (not in XDG specs as of 11/14/14)
    //Also add a fix for the location of Wine icons
    if( !DF.icon.isEmpty() ){
      QStringList sizes; sizes << "256x256" << "128x128" << "64x64" << "48x48" << "32x32" << "16x16";
      QString upath = QDir::homePath()+"/.local/share/icons/hicolor/%1/apps/%2.png";
      //qDebug() << "Wine App: Check icon" << upath;
      for(int i=0; i<sizes.length(); i++){
        if( QFile::exists(upath.arg(sizes[i],DF.icon)) ){
	  DF.icon = upath.arg(sizes[i],DF.icon);
	  //qDebug() << " - Found Icon:" << DF.icon;
	  break;
	}
      }
    }
  }
  //Return the structure
  //if (hasName && hasType) ok = true; //without Name and Type, the structure cannot be a valid .desktop file
  ok = true; //was able to open/read the file - validity determined later
  return DF;
}

bool LXDG::saveDesktopFile(XDGDesktop dFile, bool merge){
  qDebug() << "Save Desktop File:" << dFile.filePath << "Merge:" << merge;
  bool autofile = dFile.filePath.contains("/autostart/"); //use the "Hidden" field instead of the "NoDisplay"
  int insertloc = -1;
  QStringList info;
  if(QFile::exists(dFile.filePath) && merge){
    //Load the existing file and merge in in any changes
    info = LUtils::readFile(dFile.filePath);
    //set a couple flags based on the contents before we start iterating through
    // - determine if a translated field was changed (need to remove all the now-invalid translations)
    bool clearName, clearComment, clearGName; 
    QString tmp = "";
    if(!info.filter("Name=").isEmpty()){ tmp = info.filter("Name=").first().section("=",1,50); }
    clearName=(tmp!=dFile.name);
    tmp.clear();
    if(!info.filter("Comment=").isEmpty()){ tmp = info.filter("Comment=").first().section("=",1,50); }
    clearComment=(tmp!=dFile.comment);
    tmp.clear();
    if(!info.filter("GenericName=").isEmpty()){ tmp = info.filter("GenericName=").first().section("=",1,50); }
    clearGName=(tmp!=dFile.genericName);
    //Now start iterating through the file and changing fields as necessary
    bool insection = false;
    for(int i=0; i<info.length(); i++){
      if(info[i]=="[Desktop Entry]"){ 
        insection = true; 
	continue;
      }else if(info[i].startsWith("[")){ 
	if(insection){ insertloc = i; } //save this location for later insertions
	insection = false; 
	continue; 
      }
      if(!insection || info[i].isEmpty() || info[i].section("#",0,0).simplified().isEmpty()){ continue; }
      QString var = info[i].section("=",0,0);
      QString val = info[i].section("=",1,50).simplified();
      //NOTE: Clear the dFile variable as it is found/set in the file (to keep track of what has been used already)
      //    For boolian values, set them to false
      // --LOCALIZED VALUES --
      if(var.startsWith("Name")){
        if(var.contains("[") && clearName){ info.removeAt(i); i--; continue;}
	else if(!var.contains("[")){ info[i] = var+"="+dFile.name; dFile.name.clear(); }
      }else if(var.startsWith("GenericName")){
        if(var.contains("[") && clearGName){ info.removeAt(i); i--; continue;}
	else if(!var.contains("[")){ info[i] = var+"="+dFile.genericName; dFile.genericName.clear(); }
      }else if(var.startsWith("Comment")){
        if(var.contains("[") && clearComment){ info.removeAt(i); i--; continue;}
	else if(!var.contains("[")){ info[i] = var+"="+dFile.comment; dFile.comment.clear(); }

      // --STRING/LIST VALUES--
      }else if(var=="Exec"){ info[i] = var+"="+dFile.exec; dFile.exec.clear(); }
      else if(var=="TryExec"){ info[i] = var+"="+dFile.tryexec; dFile.tryexec.clear(); }
      else if(var=="Path"){ info[i] = var+"="+dFile.path; dFile.path.clear(); }
      else if(var=="Icon"){ info[i] = var+"="+dFile.icon; dFile.icon.clear(); }
      else if(var=="StartupWMClass"){ info[i] = var+"="+dFile.startupWM; dFile.startupWM.clear(); }
      else if(var=="MimeType"){ info[i] = var+"="+dFile.mimeList.join(";"); dFile.mimeList.clear(); }
      else if(var=="Categories"){ info[i] = var+"="+dFile.catList.join(";"); dFile.catList.clear(); }
      else if(var=="Keywords"){ info[i] = var+"="+dFile.keyList.join(";"); dFile.keyList.clear(); }
      else if(var=="Actions"){ info[i] = var+"="+dFile.actionList.join(";"); dFile.actionList.clear(); }
      else if(var=="OnlyShowIn"){ info[i] = var+"="+dFile.showInList.join(";"); dFile.showInList.clear(); }
      else if(var=="NotShowIn"){ info[i] = var+"="+dFile.notShowInList.join(";"); dFile.notShowInList.clear(); }
      else if(var=="URL"){ info[i] = var+"="+dFile.url; dFile.url.clear(); }
      
      // --BOOLIAN VALUES--
      else if(var=="Hidden"){ 
	if(!autofile){ info.removeAt(i); i--; continue; }
	else{ info[i] = var+"="+(dFile.isHidden ? "true": "false"); dFile.isHidden=false;}
      }else if(var=="NoDisplay"){ 
	if(autofile){ info.removeAt(i); i--; continue; }
	else{ info[i] = var+"="+(dFile.isHidden ? "true": "false"); dFile.isHidden=false;}
      }else if(var=="Terminal"){ 
	info[i] = var+"="+(dFile.useTerminal ? "true": "false"); dFile.useTerminal=false;
      }else if(var=="StartupNotify"){ 
	info[i] = var+"="+(dFile.startupNotify ? "true": "false"); dFile.startupNotify=false;
      }
      // Remove any lines that have been un-set or removed from the file
      if(info[i].section("=",1,50).simplified().isEmpty()){ info.removeAt(i); i--; }
    }
    
  }else{
    //Just write a new file and overwrite any old one 
    // (pre-set some values here which are always required)
    info << "[Desktop Entry]";
    info << "Version=1.0";
    if(dFile.type==XDGDesktop::APP){ info << "Type=Application"; }
    else if(dFile.type==XDGDesktop::LINK){ info << "Type=Link"; }
    else if(dFile.type==XDGDesktop::DIR){ info << "Type=Dir"; }
  }
  
  if(insertloc<0){ insertloc = info.size(); }//put it at the end
  //Now add in any items that did not exist in the original file
    if( !dFile.exec.isEmpty() ){ info.insert(insertloc,"Exec="+dFile.exec); }
    if( !dFile.tryexec.isEmpty() ){ info.insert(insertloc,"TryExec="+dFile.tryexec); }
    if( !dFile.path.isEmpty() ){ info.insert(insertloc,"Path="+dFile.path); }
    if( !dFile.icon.isEmpty() ){ info.insert(insertloc,"Icon="+dFile.icon); }
    if( !dFile.name.isEmpty() ){ info.insert(insertloc,"Name="+dFile.name); }
    if( !dFile.genericName.isEmpty() ){ info.insert(insertloc,"GenericName="+dFile.genericName); }
    if( !dFile.comment.isEmpty() ){ info.insert(insertloc,"Comment="+dFile.comment); }
    if( !dFile.startupWM.isEmpty() ){ info.insert(insertloc,"StartupWMClass="+dFile.startupWM); }
    if( !dFile.mimeList.isEmpty() ){ info.insert(insertloc,"MimeType="+dFile.mimeList.join(";")); }
    if( !dFile.catList.isEmpty() ){ info.insert(insertloc,"Categories="+dFile.catList.join(";")); }
    if( !dFile.keyList.isEmpty() ){ info.insert(insertloc,"Keywords="+dFile.keyList.join(";")); }
    if( !dFile.actionList.isEmpty() ){ info.insert(insertloc,"Actions="+dFile.actionList.join(";")); }
    if( !dFile.showInList.isEmpty() ){ info.insert(insertloc,"OnlyShowIn="+dFile.showInList.join(";")); }
    else if( !dFile.notShowInList.isEmpty() ){ info.insert(insertloc,"NotShowIn="+dFile.notShowInList.join(";")); }
    if( !dFile.url.isEmpty() ){ info.insert(insertloc,"URL="+dFile.url); }
    if( dFile.isHidden && autofile ){ info.insert(insertloc,"Hidden=true"); }
    else if(dFile.isHidden){ info.insert(insertloc,"NoDisplay=true"); }
    if( dFile.useTerminal){ info.insert(insertloc,"Terminal=true"); }
    if( dFile.startupNotify ){ info.insert(insertloc,"StartupNotify=true"); }
    
  //Now save the file
  return LUtils::writeFile(dFile.filePath, info, true);

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
    if(!dFile.showInList.isEmpty()){ ok = dFile.showInList.contains("Lumina", Qt::CaseInsensitive); }
    else if(!dFile.notShowInList.isEmpty()){ ok = !dFile.notShowInList.contains("Lumina",Qt::CaseInsensitive); }
    else if(dFile.name.isEmpty()){ ok = false; }
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
  if(appDirs.isEmpty()){ appDirs << "/usr/local/share" << "/usr/share" << LOS::AppPrefix()+"/share" << LOS::SysPrefix()+"/share" << L_SHAREDIR; }
  appDirs.removeDuplicates();
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
  //qDebug() << "System Application Dirs:" << out;
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

//Return the icon to use for the given category
QString LXDG::DesktopCatToIcon(QString cat){
  QString icon = "applications-other";
  if(cat=="Multimedia"){ icon = "applications-multimedia"; }
  else if(cat=="Development"){ icon = "applications-development"; }
  else if(cat=="Education"){ icon = "applications-education"; }
  else if(cat=="Game"){ icon = "applications-games"; }
  else if(cat=="Graphics"){ icon = "applications-graphics"; }
  else if(cat=="Network"){ icon = "applications-internet"; }
  else if(cat=="Office"){ icon = "applications-office"; }
  else if(cat=="Science"){ icon = "applications-science"; }
  else if(cat=="Settings"){ icon = "preferences-system"; }
  else if(cat=="System"){ icon = "applications-system"; }
  else if(cat=="Utility"){ icon = "applications-utilities"; }
  else if(cat=="Wine"){ icon = "wine"; }
  return icon;
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

QString LXDG::getDesktopExec(XDGDesktop app, QString ActionID){
  //Generate the executable line for the application
  QString out;
  QString exec = app.exec;
  if( !ActionID.isEmpty() ){
    //Go through and grab the proper exec for the listed action
    for(int i=0; i<app.actions.length(); i++){
      if(app.actions[i].ID == ActionID){
        exec = app.actions[i].exec;
        break;
      }
    }
  }
  
  if(exec.isEmpty()){ return ""; }
  else if(app.useTerminal){
    out = "xterm -lc -e "+exec;  
  }else{
    out =exec;
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
  setenv("XDG_CONFIG_DIRS","/etc/xdg:/usr/local/etc/xdg", 0);
  setenv("XDG_CACHE_HOME",QString(QDir::homePath()+"/.cache").toUtf8(), 0);
  //Don't set "XDG_RUNTIME_DIR" yet - need to look into the specs
}

QIcon LXDG::findIcon(QString iconName, QString fallback){
  //NOTE: This was re-written on 11/10/15 to avoid using the QIcon::fromTheme() framework
  //   -- Too many issues with SVG files and/or search paths with the built-in system
	
  //Check if the icon is an absolute path and exists
  bool DEBUG =false;
  if(DEBUG){ qDebug() << "[LXDG] Find icon for:" << iconName; }
  if(QFile::exists(iconName) && iconName.startsWith("/")){ return QIcon(iconName); }
  else if(iconName.startsWith("/")){ iconName.section("/",-1); } //Invalid absolute path, just look for the icon
  //Check if the icon is actually given
  if(iconName.isEmpty()){ 
    if(fallback.isEmpty()){  return QIcon(); }
    else{ return LXDG::findIcon(fallback, ""); }
  }
  //Now try to find the icon from the theme
  if(DEBUG){ qDebug() << "[LXDG] Start search for icon"; }
  //Get the currently-set theme
  QString cTheme = QIcon::themeName();
  if(cTheme.isEmpty()){ 
    QIcon::setThemeName("oxygen"); 
    cTheme = "oxygen";	  
  }
  //Make sure the current search paths correspond to this theme
  if( QDir::searchPaths("icontheme").filter("/"+cTheme+"/").isEmpty() ){
    //Need to reset search paths: setup the "icontheme" "oxygen" and "fallback" sets
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
    for(int i=0; i<paths.length(); i++){
      theme << getChildIconDirs( paths[i]+cTheme);
      oxy << getChildIconDirs(paths[i]+"oxygen"); //Lumina base icon set
      fall << getChildIconDirs(paths[i]+"hicolor"); //XDG fallback (apps add to this)
    }
    //fall << LOS::AppPrefix()+"share/pixmaps"; //always use this as well as a final fallback
    QDir::setSearchPaths("icontheme", theme);
    QDir::setSearchPaths("oxygen", oxy);
    QDir::setSearchPaths("fallback", fall);
    //qDebug() << "Setting Icon Search Paths:" << "\nicontheme:" << theme << "\noxygen:" << oxy << "\nfallback:" << fall;
  }
  //Find the icon in the search paths
  QIcon ico;
  QStringList srch; srch << "icontheme" << "oxygen" << "fallback";
  for(int i=0; i<srch.length() && ico.isNull(); i++){
    //Look for a svg first
    if(QFile::exists(srch[i]+":"+iconName+".svg") ){
        //Be careful about how an SVG is loaded - needs to render the image onto a paint device
        QSvgRenderer svg;
        if( svg.load(srch[i]+":"+iconName+".svg") ){
	  //Could be loaded - now check that it is version 1.1+ (Qt has issues with 1.0? (LibreOffice Icons) )
	  float version = 1.1; //only downgrade files that explicitly set the version as older
	  QString svginfo = LUtils::readFile(srch[i]+":"+iconName+".svg").join("\n").section("<svg",1,1).section(">",0,0);
	  svginfo.replace("\t"," "); svginfo.replace("\n"," ");
	  if(svginfo.contains(" version=")){ version = svginfo.section(" version=\"",1,1).section("\"",0,0).toFloat(); }
	  if(version>=1.1){
            ico.addFile(srch[i]+":"+iconName+".svg"); //could be loaded/parsed successfully
	  }else{
	    //qDebug() << "Old SVG Version file:" << iconName+".svg  Theme:" << srch[i];
	    //qDebug() << "SVGInfo:" << svginfo;
	  }
        }else{
          qDebug() << "Found bad SVG file:" << iconName+".svg  Theme:" << srch[i];
        }
    }
    if(QFile::exists(srch[i]+":"+iconName+".png")){
      //simple PNG image - load directly into the QIcon structure
      ico.addFile(srch[i]+":"+iconName+".png");
    }
    
  }
  //If still no icon found, look for any image format inthe "pixmaps" directory
  if(ico.isNull()){
    if(QFile::exists(LOS::AppPrefix()+"share/pixmaps/"+iconName)){
      ico.addFile(LOS::AppPrefix()+"share/pixmaps/"+iconName);
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
	  ico.addFile( pix.absoluteFilePath(found[i]) );
	  break;
	}
      }
      
    }
  }
  //Use the fallback icon if necessary
  if(ico.isNull() && !fallback.isEmpty()){
    ico = LXDG::findIcon(fallback,"");	  
  }
  if(ico.isNull()){
    qDebug() << "Could not find icon:" << iconName << fallback;
  }
  //Return the icon
  return ico;
}

QStringList LXDG::getChildIconDirs(QString parent){
  //This is a recursive function that returns the absolute path(s) of directories with *.png files
  QDir D(parent);
  QStringList out;
  QStringList dirs = D.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  if(!dirs.isEmpty() && (dirs.contains("32x32") || dirs.contains("scalable")) ){ 
    //Need to sort these directories by image size
    //qDebug() << " - Parent:" << parent << "Dirs:" << dirs;
    for(int i=0; i<dirs.length(); i++){
      if(dirs[i].contains("x")){ dirs[i].prepend( QString::number(10-dirs[i].section("x",0,0).length())+QString::number(10-dirs[i].at(0).digitValue())+"::::"); }
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
  if(multiple && !matches.isEmpty() ){ out = matches.join("::::"); }
  else if( !matches.isEmpty() ){ out = matches.first(); }
  else{ //no mimetype found - assign one (internal only - no system database changes)
    if(extension.isEmpty()){ out = "unknown/"+filename.toLower(); }
    else{ out = "unknown/"+extension.toLower(); }
  }
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
    //qDebug() << "WhiteList:" << white;
    for(int w=0; w<white.length(); w++){
      if(white[w].isEmpty()){ continue; }
      //First check for absolute paths to *.desktop file
      if( white[w].startsWith("/") ){
	 if( QFile::exists(white[w]) ){ cdefault=white[w]; break; }
	 else{ white.removeAt(w); w--; } //invalid file path - remove it from the list
      }
      //Now check for relative paths to  file (in current priority-ordered work dir)
      else if( QFile::exists(workdir+"/"+white[w]) ){ cdefault=workdir+"/"+white[w]; break; }
      //Now go through the XDG DATA dirs and see if the file is in there
      else{
	QStringList xdirs;
	  xdirs << QString(getenv("XDG_DATA_HOME"))+"/applications/";
	  tmp = QString(getenv("XDG_DATA_DIRS")).split(":");
	    for(int t=0; t<tmp.length(); t++){ xdirs << tmp[t]+"/applications/"; }
	  //Now scan these dirs
	  bool found = false;
	  //qDebug() << "Scan dirs:" << white[w] << xdirs;
	  for(int x=0; x<xdirs.length() && !found; x++){
	    if(QFile::exists(xdirs[x]+white[w])){cdefault=xdirs[x]+white[w]; found = true; }
	  }
	  if(found){ break; }
      }
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

//Find all the autostart *.desktop files
QList<XDGDesktop> LXDG::findAutoStartFiles(bool includeInvalid){
	
  //First get the list of directories to search (system first, user-provided files come later and overwrite sys files as needed)
  QStringList paths = QString(getenv("XDG_CONFIG_DIRS")).split(":");
  paths << QString(getenv("XDG_CONFIG_HOME")).split(":");
  //Now go through them and find any valid *.desktop files
  QList<XDGDesktop> files;
  QStringList filenames; //make it easy to see if this filename is an override
  QDir dir;
  for(int i=0;i<paths.length(); i++){
    if(!QFile::exists(paths[i]+"/autostart")){ continue; }	  
    dir.cd(paths[i]+"/autostart");
    QStringList tmp = dir.entryList(QStringList() << "*.desktop", QDir::Files, QDir::Name);
    for(int t=0; t<tmp.length(); t++){
      bool ok = false;
      XDGDesktop desk = LXDG::loadDesktopFile(dir.absoluteFilePath(tmp[t]), ok);
      if(!ok){ continue; } //could not read file
      //Now figure out what to do with it
      if(filenames.contains(tmp[t])){
	//This is an overwrite of a lower-priority (system?) autostart file
	// find the other file
	int old = -1;
	for(int o=0; o<files.length(); o++){
	  if(files[o].filePath.endsWith("/"+tmp[t])){ old = o; break; } //found it
	}
	if(LXDG::checkValidity(desk, false)){
	  //Full override of the lower-priority file (might be replacing exec/tryexec fields)
	  files[old] = desk;
	}else{
	  //Small override file (only the "Hidden" field listed in spec)
	  files[old].isHidden = desk.isHidden; //replace this value with the override
	  //files << desk; //still add this to the array (will be ignored/skipped later)
	}
      }else{
        //This is a new autostart file
	files << desk;
	filenames << tmp[t];
      }
    }//end of loop over *.desktop files
  } //end of loop over directories
  
  //Now filter the results by validity if desired
  if(!includeInvalid){
    for(int i=0; i<files.length(); i++){
      if( !LXDG::checkValidity(files[i], false) || files[i].isHidden ){
        //Invalid file - go ahead and remove it from the output list
	files.removeAt(i);
	i--;
      }
    }
  }
	
  return files;
}

bool LXDG::setAutoStarted(bool autostart, XDGDesktop app){
  //First get the list of system directories to search (system first, user-provided files come later and overwrite sys files as needed)
  QStringList paths = QString(getenv("XDG_CONFIG_DIRS")).split(":");
  QString upath = QString(getenv("XDG_CONFIG_HOME")).section(":",0,0);
  if(upath.isEmpty()){ upath = QDir::homePath()+"/.config/autostart/"; }
  else{ upath.append("/autostart/"); }
  //Quick check/finish for user-defined files which are getting disabled (just remove the file)
  if(app.filePath.startsWith(upath) && !autostart){
    return QFile::remove(app.filePath);
  }
  bool sysfile = false;
  for(int i=0; i<paths.length(); i++){
    if(app.filePath.startsWith(paths[i]+"/autostart/") ){
      sysfile = true;
      //Change it to the user-modifiable directory
      app.filePath = app.filePath.replace(paths[i]+"/autostart/", upath);
    }
  }
  //Make sure the user-autostart dir is specified, and clean the app structure as necessary
  if( !app.filePath.startsWith(upath) && autostart){ 
    //Some other non-override autostart file - set it up to open with lumina-open
    if(!app.filePath.endsWith(".desktop")){
      app.exec = "lumina-open \""+app.filePath+"\"";
      app.tryexec = app.filePath; //make sure this file exists
      if(app.name.isEmpty()){ app.name = app.filePath.section("/",-1); }
      if(app.icon.isEmpty()){ app.icon = LXDG::findAppMimeForFile(app.filePath); app.icon.replace("/","-"); }
      app.filePath = upath+app.filePath.section("/",-1)+".desktop";
      app.type = XDGDesktop::APP;
    }else{
      //Some other *.desktop file on the system (keep almost all the existing settings/values)
      // - setup a redirect to the other file
      app.exec = "lumina-open \""+app.filePath+"\"";
      app.tryexec = app.filePath; //make sure this file exists
      // - Adjust the actual path where this file will get saved
      app.filePath = upath+app.filePath.section("/",-1);
    }
  }
  //Now save the "hidden" value into the file
  app.isHidden = !autostart; //if hidden, it will not be autostarted
  //Now save the file as necessary
  bool saved = false;
  //qDebug() << " - Saving AutoStart File:" << app.filePath << app.name << app.isHidden;
  if(sysfile){
    //Just an override file for the "hidden" field - nothing more
    QStringList info;
      info << "[Desktop Entry]" << "Type=Application" << QString("Hidden=")+ (app.isHidden ? QString("true"): QString("false"));
    saved = LUtils::writeFile(app.filePath, info, true);
  }else{
    //Need to actually save the full file
    saved = LXDG::saveDesktopFile(app);
  }
  return saved;
}

bool LXDG::setAutoStarted(bool autostart, QString filePath){
  //Convenience function for the auto-start setter
  XDGDesktop desk;
  if(filePath.endsWith(".desktop")){
    bool ok = false;
    desk = LXDG::loadDesktopFile(filePath, ok);
    if(!ok){ return false; } //error reading input file
  }else{
    desk.filePath = filePath;
    desk.useTerminal = false;
  }
  return LXDG::setAutoStarted(autostart, desk);
}
