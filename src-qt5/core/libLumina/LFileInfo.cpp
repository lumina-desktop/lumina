//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LFileInfo.h"
#include <LUtils.h>
#include <QRegExp>
#include <unistd.h>

LFileInfo::LFileInfo() : QFileInfo(){
  desk = 0;
  c_uid = -1;
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
  c_uid = geteuid();
  //Now load the extra information
  QString abspath = this->absoluteFilePath();
  if( this->isDir() ){
    mime = "inode/directory";
    //Special directory icons
    QString name = this->fileName().toLower();
    if(name=="desktop" || abspath == LUtils::standardDirectory(LUtils::Desktop, false) ){ iconList << "user-desktop"; }
    else if(name=="tmp"){ iconList << "folder-temp"; }
    else if(name=="video" || name=="videos" || abspath == LUtils::standardDirectory(LUtils::Videos, false)){ iconList << "folder-video" << "camera-photo-film" ; }
    else if(name=="music" || name=="audio" || abspath == LUtils::standardDirectory(LUtils::Music, false)){ iconList << "folder-sound" << "media-playlist-audio"; }
    else if(name=="projects" || name=="devel"){ iconList << "folder-development"; }
    else if(name=="notes"){ iconList << "folder-txt" << "note-multiple-outline" << "note-multiple"; }
    else if(name=="downloads" || abspath == LUtils::standardDirectory(LUtils::Downloads, false)){ iconList << "folder-downloads" << "folder-download"; }
    else if(name=="documents" || abspath == LUtils::standardDirectory(LUtils::Documents, false)){ iconList << "folder-documents"; }
    else if(name=="images" || name=="pictures" || abspath == LUtils::standardDirectory(LUtils::Pictures, false)){ iconList << "folder-image"; }
    else if(this->absoluteFilePath().startsWith("/net/")){ iconList << "folder-remote"; }
    else if( !this->isReadable() ){ iconList << "folder-locked"<< "folder-lock"; }
    iconList << "folder";
  }else if( this->suffix()=="desktop"){
    mime = "application/x-desktop";
    desk = new XDGDesktop(this->absoluteFilePath(), 0);
    if(desk->type!=XDGDesktop::BAD){
      //use the specific desktop file info (if possible)
      if(!desk->icon.isEmpty()){ iconList << desk->icon; }
    }
  }else{
    //Generic file, just determine the mimetype
    mime = LXDG::findAppMimeForFile(this->fileName());
  }
  //check the mimetype for an icon as needed
  QString tmp = mime;
    tmp.replace("/","-");
    iconList << tmp;
  if(this->isExecutable()){
    iconList << "application-x-executable";
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
    else{
      zfs_ds = out.section("\n",0,0).section("\t",0,0).simplified();
      zfs_dspath = this->canonicalFilePath().section(zfs_ds.section("/",1,-1), 1,-1); //relative path
      if(!zfs_dspath.isEmpty()){ zfs_dspath.prepend(zfs_ds); }
      else{ zfs_dspath = zfs_ds; } //on the current dataset "root"
      //qDebug() << "Got ZFS dataset:" << zfs_ds << zfs_dspath;
    }
    //qDebug() << "Found Dataset:" << zfs_ds << out << ok;
    if(ok){
      //Also get the list of permissions this user has for modifying the ZFS dataset
      QStringList perms = LUtils::runCommand(ok, "zfs", QStringList() << "allow" << zfs_ds ).split("\n");
      //qDebug() << "Permissions:" << perms;
      if(!perms.isEmpty() && ok){
        //Now need to filter/combine the permissions for all the groups this user is a part of
        QStringList gplist = LUtils::runCommand(ok, "id", QStringList() << "-np").split("\n").filter("groups");
        if(!gplist.isEmpty()){ gplist = gplist.first().replace("\t", " ").split(" ",QString::SkipEmptyParts); gplist.removeAll("groups"); }
        for(int i=0; i<gplist.length(); i++){
          QStringList tmp = perms.filter(QRegExp("[user|group] "+gplist[i]));
          if(tmp.isEmpty()){ continue; }
          zfs_perms << tmp.first().section(" ",2,2,QString::SectionSkipEmpty).split(",",QString::SkipEmptyParts);
        }
        zfs_perms.removeDuplicates();
        //qDebug() << "Got ZFS Permissions:" << zfs_ds << zfs_perms;
      }
    }

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
  //Go through the icon list and find the first one that exists in the current theme
  for(int i=0; i<iconList.length(); i++){
    if( QIcon::hasThemeIcon(iconList[i])){ return iconList[i]; }
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

bool LFileInfo::isZfsDataset(QString path){
  if(!path.isEmpty() && zfsAvailable() ){
    //manual check of a dir
    return (0 == LUtils::runCmd("zfs", QStringList() << "get" << "-H" << "atime" << path) );
  }
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

//ZFS Permissions/Modifications
bool LFileInfo::canZFScreate(){
  if(!goodZfsDataset()){ return false; }
  return (zfs_perms.contains("create")  || (c_uid==0) );
}

bool LFileInfo::zfsCreateDataset(QString subdir){
  if(!canZFScreate()){ return false; }
  if(subdir.startsWith("/")){ qDebug() << "Not a relative path!!"; return false; }
  if( QFile::exists(this->canonicalFilePath()+"/"+subdir) ){
    return false;
  }
  bool ok = false;
  QString info = LUtils::runCommand(ok, "zfs", QStringList() << "create" << zfs_dspath+"/"+subdir );
  if(!ok){ qDebug() << "Error Creating ZFS Dataset:" << subdir << info; }
  return ok;
}

bool LFileInfo::canZFSdestroy(){
  if(!goodZfsDataset()){ return false; }
  return (zfs_perms.contains("destroy") || (c_uid==0) );
}

bool LFileInfo::zfsDestroyDataset(QString subdir){
  if(!canZFSdestroy()){ return false; }
  if(!subdir.isEmpty() && !subdir.startsWith("/")){
    if( isZfsDataset(this->canonicalFilePath()+"/"+subdir) ){ subdir = zfs_dspath+"/"+subdir; }
  }
  else if(subdir.isEmpty() && (zfs_ds == zfs_dspath) ){ subdir = zfs_ds; }
  else{ qDebug() << "Invalid subdir:" << subdir; return false; }
  bool ok = false;
  QString info = LUtils::runCommand(ok, "zfs", QStringList() << "destroy" << subdir);
  if(!ok){ qDebug() << "Error Destroying ZFS Dataset:" << subdir << info; }
  return ok;
}

bool LFileInfo::zfsDestroySnapshot(QString snapshot){
  if(!canZFSdestroy()){ return false; }
  bool ok = false;
  QString info = LUtils::runCommand(ok, "zfs", QStringList() << "destroy" << zfs_ds+"@"+snapshot);
  if(!ok){ qDebug() << "Error Destroying ZFS Snapshot:" << snapshot << info; }
  return ok;
}

bool LFileInfo::canZFSclone(){
  if(!goodZfsDataset()){ return false; }
  return (zfs_perms.contains("clone")  || (c_uid==0) );
}

bool LFileInfo::zfsCloneDataset([[maybe_unused]] QString subdir, [[maybe_unused]] QString newsubdir){
  if(!canZFSclone()){ return false; }

  return false;
}

bool LFileInfo::canZFSsnapshot(){
  if(!goodZfsDataset()){ return false; }
  return (zfs_perms.contains("snapshot")  || (c_uid==0) );
}

bool LFileInfo::zfsSnapshot(QString snapname){
  if(!canZFSsnapshot()){ return false; }
  bool ok = false;
  QString info = LUtils::runCommand(ok, "zfs", QStringList() << "snapshot" << zfs_ds+"@"+snapname);
  if(!ok){ qDebug() << "Error Creating ZFS Snapshot:" << snapname << info; }
  return ok;
}

bool LFileInfo::canZFSrollback(){
  if(!goodZfsDataset()){ return false; }
  return (zfs_perms.contains("rollback")  || (c_uid==0) );
}

bool LFileInfo::zfsRollback(QString snapname){
  if(!canZFSrollback()){ return false; }
  bool ok = false;
  QString info = LUtils::runCommand(ok, "zfs", QStringList() << "rollback" << zfs_ds+"@"+snapname);
  if(!ok){ qDebug() << "Error Rolling back to ZFS Snapshot:" << snapname << info; }
  return ok;
}
