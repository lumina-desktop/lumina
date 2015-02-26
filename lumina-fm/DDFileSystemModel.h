//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is a simple subclassed QFileSystemModel to enable drag and drop 
// (and moving) but disable all the other filesystem modifications
//===========================================
#ifndef _LUMINA_FILE_MANAGER_DDFILESYSTEMMODEL_H
#define _LUMINA_FILE_MANAGER_DDFILESYSTEMMODEL_H

#include <QFileSystemModel>
#include <QObject>

class DDFileSystemModel : public QFileSystemModel{
	Q_OBJECT
public:
	DDFileSystemModel(QObject *parent = 0) : QFileSystemModel(parent){
	  this->setReadOnly(false); //need this to enable DnD
	}
	~DDFileSystemModel(){}
		
	virtual Qt::ItemFlags flags(const QModelIndex &index) const {
	  //First get all the flags from the standard QFileSystemModel
	  Qt::ItemFlags defaultflags = QFileSystemModel::flags(index);
	  //Now if it has the "Editable" flag set - remove it
	  if(defaultflags & Qt::ItemIsEditable){
	    defaultflags ^= Qt::ItemIsEditable;
	  }
		  
	  return defaultflags;
	}
};

#endif