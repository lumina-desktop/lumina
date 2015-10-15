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

#define ZSNAPDIR QString("/.zfs/snapshot/")

#define DIR_DEBUG 0

//Class used for keeping track of directory information in the HASH
class LDirInfoList{
public:
	//Internal variables
	QDateTime lastcheck;
	QList<LFileInfo> list;
	QStringList fileNames; //list of filenames for comparison/checking sorting
	QString dirpath; //directory this structure was reading
	QString snapdir; //base snapshot directory (if one was requested/found)
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
	  if(dirpath.contains(ZSNAPDIR) && snapdir.isEmpty()){
	    snapdir = dirpath.section(ZSNAPDIR,0,0)+ZSNAPDIR; //no need to go looking for it later
	  }
	  QFileInfoList dirlist;
	  //Fill the structure
	  list.clear();
	  fileNames.clear();
	  lastcheck = QDateTime::currentDateTime().addMSecs(-500); //prevent missing any simultaneous dir changes
	  if(showhidden){ dirlist = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden , QDir::Name | QDir::DirsFirst); }
	  else{ dirlist = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot , QDir::Name | QDir::DirsFirst); }
	  //Simple add routine - can make it more dynamic/selective about updating individual items later
	    for(int i=0; i<dirlist.length(); i++){
	      list << LFileInfo(dirlist[i]); //generate the extra information for this file
	      fileNames << dirlist[i].fileName(); //add the filename to the list
	    }	
	}
	
	void findSnapDir(){
	  //Search the filesystem 
	  if(dirpath.contains(ZSNAPDIR)){
	    snapdir = dirpath.section(ZSNAPDIR,0,0)+ZSNAPDIR; //no need to go looking for it
	  }else{
	    //Need to backtrack 
	    QDir dir(dirpath);
	    bool found = false;
	    while(dir.canonicalPath()!="/" && !found){
	      //qDebug() << " -- Checking for snapshot dir:" << dir.canonicalPath();
	      if(dir.exists(".zfs/snapshot")){
		snapdir = dir.canonicalPath()+ZSNAPDIR;
		found = true;
	      }else{ 
		dir.cdUp(); 
	      }
	    }//end loop 
	  }
	}

};

//This class is designed to be run in a background thread and get all the necessary info for a directory listing
class DirData : public QObject{
	Q_OBJECT
private:
	QHash<QString, LDirInfoList> HASH; //Where we cache any info for rapid access later

signals:
	void DirDataAvailable(QString, QString, LFileInfoList); //[ID, Dirpath, DATA]
	void SnapshotDataAvailable(QString, QString, QStringList); //[ID, BaseSnapDir, SnapNames]

public:
	//Variables
	bool showHidden; //Whether hidden files/dirs should be output
	bool zfsavailable; //Whether it should even bother looking for ZFS snapshots

	//Functions
	DirData(){ 
	  showHidden = false; 
	  zfsavailable = false;
	}
	~DirData(){}

public slots:
	void GetDirData(QString ID, QString dirpath){ 
	  if(DIR_DEBUG){ qDebug() << "GetDirData:" << ID << dirpath; }
	  //The ID is used when returning the info in a moment
	  //Make sure to use the canonical path in the HASH search - don't use 
	  QString canon = QFileInfo(dirpath).canonicalFilePath();
	  if(!HASH.contains(canon)){
	    //New directory (not previously loaded)
	    LDirInfoList info(canon);
	      info.update(showHidden);
	    HASH.insert(canon, info);
	  }else{
	    //See if the saved info needs to be updated
	    //if( (HASH.value(canon).hashidden != showHidden) || (QFileInfo(canon).lastModified() > HASH.value(canon).lastcheck) ){
	      HASH[canon].update(showHidden);
	    //}
	  }
	  if(DIR_DEBUG){ qDebug() << " -- Dir Data Found:" << ID << dirpath << HASH.value(canon).list.length(); }
	  emit DirDataAvailable(ID, dirpath, HASH.value(canon).list);
	}
	
	void GetSnapshotData(QString ID, QString dirpath){
	  if(DIR_DEBUG){ qDebug() << "GetSnapshotData:" << ID << dirpath; }
	  QString base; QStringList snaps;
	  //Only check if ZFS is flagged as available
	  if(zfsavailable){
	    //First find if the hash already has an entry for this directory
	    if(false){ //!HASH.contains(dirpath)){
	      LDirInfoList info(dirpath);
	      HASH.insert(dirpath,info);
	    }
	    //Now see if a snapshot directory has already been located
	    if(HASH.value(dirpath).snapdir.isEmpty()){
	      HASH[dirpath].findSnapDir();
	    }
	    //Now read off all the available snapshots
	    if(HASH.value(dirpath).snapdir != "-"){
	      //Good snapshot directory found - read off the current snapshots (can change regularly - don't cache this)
	      base = HASH.value(dirpath).snapdir;
	      QDir dir(base);
	      snaps = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time |QDir::Reversed );
	      //Also remove any "empty" snapshots (might be leftover by tools like "zfsnap")
	      for(int i=0; i<snaps.length(); i++){
		dir.cd(base+"/"+snaps[i]);
		//qDebug() << "Snapshot Dir Contents Count:" << dir.count() << snaps[i];
	        if(dir.count() < 3 && dir.exists() ){ snaps.removeAt(i); i--; } // "." and ".." are always in the count
	      }
	      //NOTE: snaps are sorted oldest -> newest
	    }
	    
	  }
	  //if(DIR_DEBUG){ qDebug() << " -- Snap Data Found:" << ID << base << snaps; }
	  emit SnapshotDataAvailable(ID, base, snaps); 
	}
	
};

#endif
