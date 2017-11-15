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
  this->setFile(filepath);
  loadExtraInfo();
}

LFileInfo::LFileInfo(QFileInfo info) : QFileInfo(){ //overloaded contructor
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
