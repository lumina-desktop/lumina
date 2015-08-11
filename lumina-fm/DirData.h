//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the backend classes for fetching directory information/contents
//===========================================
#ifndef _LUMINA_FM_BACKGROUND_DATA_CLASSES_H
#define _LUMINA_FM_BACKGROUND_DATA_CLASSES_H

#include <QObject>
#include <QList>
#include <QString>
#include <QFileInfo>
#include  <QDir>

#include <LuminaXDG.h>
#include <LuminaUtils.h>

//Need some extra information not usually available by a QFileInfo
class LFileInfo : public QFileInfo{
private:
	QString mime, icon;
	XDGDesktop desk;

	void loadExtraInfo(){
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
	
public:
	LFileInfo(QString filepath){ //overloaded contructor
	  this->setFile(filepath);
	  loadExtraInfo();
	}	
	LFileInfo(QFileInfo info){ //overloaded contructor
	  this->swap(info); //use the given QFileInfo without re-loading it
	  loadExtraInfo();
	}		
	~LFileInfo(){}
	
	//Functions for accessing the extra information
	// -- Return the mimetype for the file
	QString mimetype(){
	  return mime;
	}
	
	// -- Return the icon to use for this file
	QString iconfile(){
	  return (icon.isEmpty() ? mime: icon); //Fall back on the mimetype if no icon listed
	}
	
	// -- Check if this is an XDG desktop file
	bool isDesktopFile(){
	  return (!desk.filePath.isEmpty());	
	}
	
	// -- Allow access to the XDG desktop data structure
	const XDGDesktop* XDG(){
	  return &desk;
	}
	
	// -- Check if this is a readable image file (for thumbnail support)
	bool isImage(){
	  if(!mime.startsWith("image/")){ return false; } //quick return for non-image files
	  //Check the Qt subsystems to see if this image file can be read
	  return ( !LUtils::imageExtensions().filter(this->suffix().toLower()).isEmpty() );
	}
};

class LDirInfoList{
public:
	//Internal variables
	QDateTime lastcheck;
	QList<LFileInfo> list;
	QStringList fileNames; //list of filenames for comparison/checking sorting
	QString dirpath; //directory this structure was reading
	bool hashidden;

	//Access Functions
	LDirInfoList(QString path = ""){
	  dirpath = path;
	  list.clear();
	  fileNames.clear();
	  hashidden = false;
	}
	~LDirInfoList(){}

	//(re)Load a directory contents
	void update(bool showhidden = false){
	  if(dirpath.isEmpty()){ return; } //nothing to do
	  //Assemble the structures
	  QDir dir(dirpath);
	  hashidden = showhidden;
	  if(!dir.exists()){
	    list.clear();
	    fileNames.clear();
	    dirpath.clear(); //invalid directory now
	    return;
	  }
	  QFileInfoList dirlist;
	  //Fill the structure
	  list.clear();
	  fileNames.clear();
	  lastcheck = QDateTime::currentDateTime();
	  if(showhidden){ dirlist = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden , QDir::Name | QDir::DirsFirst); }
	  else{ dirlist = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot , QDir::Name | QDir::DirsFirst); }
	  //Simple add routine - can make it more dynamic/selective about updating individual items later
	    for(int i=0; i<dirlist.length(); i++){
	      list << LFileInfo(dirlist[i]); //generate the extra information for this file
	      fileNames << dirlist[i].fileName(); //add the filename to the list
	    }	
	  
	}

};

//This class is designed to be run in a background thread and get all the necessary info for a directory listing
class DirData : public QObject{
	Q_OBJECT
private:
	QHash<QString, LDirInfoList> HASH; //Where we cache any info for rapid access later

signals:
	void DirDataAvailable(QString, QList<LFileInfo>); //[ID, DATA]

public:
	//Variables
	bool showHidden;

	//Functions
	DirData(){ 
	  showHidden = false; 
	}
	~DirData(){}

public slots:
	void GetDirData(QString ID, QString dirpath){ 
	  //The ID is used when returning the info in a moment
	  if(!HASH.contains(dirpath)){
	    //New directory (not previously loaded)
	    LDirInfoList info(dirpath);
	      info.update(showHidden);
	    HASH.insert(dirpath, info);
	  }else{
	    //See if the saved info needs to be updated
	    if( (HASH.value(dirpath).hashidden != showHidden) || (QFileInfo(dirpath).lastModified() > HASH.value(dirpath).lastcheck) ){
	      HASH[dirpath].update(showHidden);
	    }
	    
	  }
	  emit DirDataAvailable(ID, HASH.value(dirpath).list);
	}
	
};

#endif
