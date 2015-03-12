//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the Icon provider for files based on mime types
//===========================================
#ifndef _LUMINA_FILE_MANAGER_ICON_PROVIDER_H
#define _LUMINA_FILE_MANAGER_ICON_PROVIDER_H

#include <QFileIconProvider>
#include <QIcon>
#include <QString>
#include <QFileInfo>

#include <LuminaXDG.h>

class MimeIconProvider : public QFileIconProvider{

public:
	bool showthumbnails;
	MimeIconProvider() : QFileIconProvider(){
	  showthumbnails = false;
	}
	~MimeIconProvider(){}
	

		
	QIcon icon(const QFileInfo &info) const{
	  if(info.isDir()){
	    return LXDG::findIcon("folder","");
	  }else if(info.isFile()){
	    if(showthumbnails && (info.suffix().toLower()=="png" || info.suffix().toLower()=="jpg") ){
	      //make sure to only load small versions of the files into memory: could have hundreds of them...
	      return QIcon( QPixmap(info.absoluteFilePath()).scaledToHeight(64) );
	    }else if(info.fileName().endsWith(".desktop") ){
	      bool ok = false;
	      XDGDesktop desk = LXDG::loadDesktopFile(info.absoluteFilePath(), ok);
	      if(ok){
	        return LXDG::findIcon(desk.icon, "unknown");
	      }else{
		return LXDG::findMimeIcon(info.fileName());
	      }
	    }else{
	      return LXDG::findMimeIcon(info.fileName());
	    }
	  }else{
	    return LXDG::findIcon("unknown","");
	  }
	}
	
	QString type(const QFileInfo &info) const{
	  if(info.isDir()){
	    return QObject::tr("Directory");
	  }else if(info.completeBaseName().isEmpty() || info.suffix().isEmpty() ){
	    return QObject::tr("Unknown"); //hidden file without an extension
	  }else if(info.suffix()=="desktop"){
	    return QObject::tr("Application");
	  }else{
	    return info.suffix().toUpper();
	  }
	}
	
};

#endif
