//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This menu is used to automatically generate menu contents 
//    based on the JSON output of an external script/utility
//===========================================
#ifndef _LUMINA_DESKTOP_JSON_MENU_H
#define _LUMINA_DESKTOP_JSON_MENU_H

#include <QMenu>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <LUtils.h>
#include <LuminaXDG.h>
#include "LSession.h"

class JsonMenu : public QMenu{
	Q_OBJECT
private:
	QString exec;

public:
	JsonMenu(QString execpath, QWidget *parent = 0) : QMenu(parent){
	  exec = execpath;
	  connect(this, SIGNAL(aboutToShow()), this, SLOT(updateMenu()) );
	  connect(this, SIGNAL(triggered(QAction*)), this, SLOT(itemTriggered(QAction*)) );
	}

private slots:
	void parseObject(QString label, QJsonObject obj){
	  if( label.isEmpty() || !obj.contains("type") ){ return; }
          QString type = obj.value("type").toString();
	  if(type.toLower()=="item"){
	    QAction *act = this->addAction(label);
	    if(obj.contains("icon")){ act->setIcon( LXDG::findIcon(obj.value("icon").toString(),"") ); }
	    if(obj.contains("action")){ act->setWhatsThis( obj.value("action").toString() ); }
	    else{ act->setEnabled(false); } //not interactive
	  }else if(type.toLower()=="menu"){
	    
	  }else if(type.toLower()=="jsonmenu"){
	    //This is a recursive JSON menu object
	    if(!obj.contains("exec")){ return; }
	    JsonMenu *menu = new JsonMenu(obj.value("exec").toString(), this);
	      menu->setTitle(label);
	      if(obj.contains("icon")){ menu->setIcon(LXDG::findIcon(obj.value("icon").toString(),"") ); }
	    this->addMenu(menu);
	  }
	}

	void updateMenu(){
          this->clear();
	  QJsonDocument doc = QJsonDocument::fromJson( LUtils::getCmdOutput(exec).join(" ").toLocal8Bit() );
          if(doc.isNull() || !doc.isObject()){
	    this->addAction( QString(tr("Error parsing script output: %1")).arg("\n"+exec) )->setEnabled(false);
	  }else{
	    QStringList keys = doc.object().keys();
	    for(int i=0; i<keys.length(); i++){
	      if(doc.object().value(keys[i]).isObject()){
	        parseObject(keys[i], doc.object().value(keys[i]).toObject());
	      }
	    }
	  }
	}

	void itemTriggered(QAction *act){
	  if(act->parent()!=this || act->whatsThis().isEmpty() ){ return; } //only handle direct child actions - needed for recursive nature of menu
	  QString cmd = act->whatsThis();
	  QString bin = cmd.section(" ",0,0);
	  if( !LUtils::isValidBinary(bin) ){ cmd.prepend("lumina-open "); }
	  LSession::handle()->LaunchApplication(cmd);
	}
};
#endif
