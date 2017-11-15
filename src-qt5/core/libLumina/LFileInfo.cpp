//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LFileInfo.h"
#include <LUtils.h>

LFileInfo::LFileInfo() : QFileInfo(){
  desk = 0;
}

LFileInfo::LFileInfo(QString filepath) : QFileInfo(){ //overloaded contructor
  desk = 0;
  this->setFile(filepath);
  loadExtraInfo();
}

LFileInfo::LFileInfo(QFileInfo info) : QFileInfo(){ //overloaded contructor
  desk = 0;
  this->swap(info); //use the given QFileInfo without re-loading it
  loadExtraInfo();
}
LFileInfo::~LFileInfo(){
  if(desk!=0){ desk->deleteLater(); }
}

//Need some extra information not usually available by a QFileInfo
void LFileInfo::loadExtraInfo(){
  if(desk!=0){ desk->deleteLater(); }
  desk = 0;
  //Now load the extra information
  if(this->absoluteFilePath().startsWith("/net/") || this->isDir() ){
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
    else if(this->absoluteFilePath().startsWith("/net/")){ icon = "folder-remote"; }
    else if( !this->isReadable() ){ icon = "folder-locked"; }
  }else if( this->suffix()=="desktop"){
    mime = "application/x-desktop";
    icon = "application-x-desktop"; //default value
    desk = new XDGDesktop(this->absoluteFilePath(), 0);
    if(desk->type!=XDGDesktop::BAD){
      //use the specific desktop file info (if possible)
      if(!desk->icon.isEmpty()){ icon = desk->icon; }
    }
  }else{
    //Generic file, just determine the mimetype
    mime = LXDG::findAppMimeForFile(this->fileName());
  }
}

bool LFileInfo::zfsAvailable(){
  static unsigned int avail = 2;
  if(avail == 2){ avail = (LUtils::isValidBinary("zfs") ? 0 : 1); }
  return (avail == 0);
}

void LFileInfo::getZfsDataset(){
  if(zfs_ds.isEmpty()){
    //First run - need to probe the current directory
    bool ok = false;
    //Use the "atime" property for this check - been around since the earliest versions of ZFS and should take no time to probe
    QString out = LUtils::runCommand(ok, "zfs", QStringList() << "get" << "-H" << "atime" << this->canonicalFilePath() );
    if(!ok){ zfs_ds = "."; } //just something that is not empty - but is clearly not a valid dataset
    else{  zfs_ds = out.section("\n",0,0).section("\t",0,0).simplified(); }
    //qDebug() << "Found Dataset:" << zfs_ds << out << ok;
  }
}

bool LFileInfo::goodZfsDataset(){
  if(!zfsAvailable()){ return false; }
  getZfsDataset(); //ensure this field is populated
  if(zfs_ds=="." || zfs_ds.isEmpty()){ return false; }
  return true;
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
  }else if(!mime.isEmpty()){
    QString tmp = mime;
    tmp.replace("/","-");
    return tmp;
  }else if(this->isExecutable()){
    return "application-x-executable";
  }
  return ""; //Fall back to nothing
}

// -- Check if this is an XDG desktop file
bool LFileInfo::isDesktopFile(){
  if(desk==0){ return false; }
  return (!desk->filePath.isEmpty());
}

// -- Allow access to the XDG desktop data structure
XDGDesktop* LFileInfo::XDG(){
  return desk;
}

// -- Check if this is a readable video file (for thumbnail support)
bool LFileInfo::isVideo(){
  if(!mime.startsWith("video/")){ return false; }
  //Check the hardcoded list of known supported video formats to see if the thumbnail can be generated
  return ( !LUtils::videoExtensions().filter(this->suffix().toLower()).isEmpty() );
}

// -- Check if this is a readable image file
bool LFileInfo::isImage(){
  if(!mime.startsWith("image/")){ return false; } //quick return for non-image files
  //Check the Qt subsystems to see if this image file can be read
  return ( !LUtils::imageExtensions().filter(this->suffix().toLower()).isEmpty() );
}

bool LFileInfo::isAVFile(){
  return (mime.startsWith("audio/") || mime.startsWith("video/") );
}

bool LFileInfo::isZfsDataset(){
  if(!goodZfsDataset()){ return false; }
  return ( ("/"+zfs_ds.section("/",1,-1)) == this->canonicalFilePath());
}

QString LFileInfo::zfsPool(){
  if(!goodZfsDataset()){ return ""; }
  return zfs_ds.section("/",0,0);
}

QStringList LFileInfo::zfsSnapshots(){
  if(!goodZfsDataset()){ return QStringList(); }
  QString relpath = this->canonicalFilePath().remove(0, QString("/"+zfs_ds.section("/",1,-1)).length() );
  //qDebug() << "Got Relative path:" << zfs_ds << this->canonicalFilePath() << relpath;
  QDir dir("/"+zfs_ds.section("/",1,-1)+"/.zfs/snapshot/");
  QStringList snaps = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
  for(int i=0; i<snaps.length(); i++){
    if( QFile::exists(dir.absoluteFilePath(snaps[i])+relpath) ){  snaps[i].append("::::" + dir.absoluteFilePath(snaps[i])+relpath ); }
    else{ snaps.removeAt(i); i--; }
  }
  return snaps;
}

QJsonObject LFileInfo::zfsProperties(){
  QJsonObject props;
  if(!goodZfsDataset()){ return props; }
  bool ok = false;
  QStringList out = LUtils::runCommand(ok, "zfs", QStringList() << "get" << "-H" << "all"  << zfs_ds).split("\n");
  //Note: Formating of zfs output: tab-delimited, with columns [dataset, property, value, source]
  for(int i=0; i<out.length() && ok; i++){
    if(out[i].simplified().isEmpty()){ continue; }
    QJsonObject prop;
    prop.insert("property", out[i].section("\t",1,1).simplified());
    prop.insert("value", out[i].section("\t",2,2).simplified());
    prop.insert("source", out[i].section("\t",3,-1).simplified());
    props.insert(prop.value("property").toString(), prop);
  }
  return props;
}

bool LFileInfo::zfsSetProperty(QString property, QString value){
  if(!goodZfsDataset()){ return false; }
  bool ok = false;
  QString info = LUtils::runCommand(ok, "zfs", QStringList() << "set" << property+"="+value << zfs_ds);
  if(!ok){ qDebug() << "Error Setting ZFS Property:" << property+"="+value << info; }
  return ok;
}
