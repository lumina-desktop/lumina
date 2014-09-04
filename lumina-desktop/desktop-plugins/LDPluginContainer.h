//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class is the generic container for a desktop plugin that handles
//    saving/restoring all the movement and sizing
//===========================================
#ifndef _LUMINA_DESKTOP_DESKTOP_PLUGIN_CONTAINER_H
#define _LUMINA_DESKTOP_DESKTOP_PLUGIN_CONTAINER_H

#include <QObject>
#include <QMdiSubWindow>
#include <QSettings>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QString>
#include <QFile>

#include "LDPlugin.h"

class LDPluginContainer : public QMdiSubWindow{
	Q_OBJECT
	
private:
	QSettings *settings;
	bool locked, setup;
	
public:
	LDPluginContainer(LDPlugin *plugin = 0, bool islocked = true) : QMdiSubWindow(){
	  locked = islocked;
	  setup=true;
	  this->setWhatsThis(plugin->ID());
	  if(locked){ this->setWindowFlags(Qt::FramelessWindowHint); }
	  else{ this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint); }
	  settings = plugin->settings; //save this pointer for access later
	  if(settings->allKeys().isEmpty()){
	    //Brand new plugin - no location/size info saved yet
	    //save the initial size of the plugin - the initial location will be set automatically
	      settings->setValue("location/width", plugin->sizeHint().width());
	      settings->setValue("location/height", plugin->sizeHint().height());
	      settings->sync();
	  }
	  this->setContentsMargins(0,0,0,0);
	  if(!locked){
	    //this->setWindowTitle( plugin->ID().replace("---"," - ") );
	    //this->setWidget( new QWidget() );
	    this->setWidget( plugin );
	  }else{
	    this->setStyleSheet("LDPluginContainer{ background: transparent; border: none;}");
	    this->setWidget(plugin);
	  }
	}
	
	~LDPluginContainer(){
	}

	void loadInitialPosition(){
	  QRect set(settings->value("location/x",-12345).toInt(), settings->value("location/y",-12345).toInt(), settings->value("location/width",this->widget()->sizeHint().width()).toInt(), settings->value("location/height",this->widget()->sizeHint().height()).toInt());
	  //qDebug() << "Initial Plugin Location:" << set.x() << set.y() << set.width() << set.height();
	    if(set.height() < 10){ set.setHeight(10); } //to prevent foot-shooting
	    if(set.width() < 10){ set.setWidth(10); } //to prevent foot-shooting
	    if(set.x()!=-12345 && set.y()!=-12345){
	      //custom location specified
	      this->setGeometry(set);
	    }else{
	      this->resize(set.width(), set.height());
	    }
	  setup=false; //done with setup
	}

signals:
	void PluginRemoved(QString);
	
protected:
	void moveEvent(QMoveEvent *event){
	  //Save this location to the settings
	  if(!locked && !setup){
	    //qDebug() << "DP Move:" << event->pos().x() << event->pos().y();
	    settings->setValue("location/x", event->pos().x());
	    settings->setValue("location/y", event->pos().y());
	    settings->sync();
	  }
	}
	
	void resizeEvent(QResizeEvent *event){
	  //Save this size info to the settings
	  if(!locked && !setup){
	    //qDebug() << "DP Resize:" << event->size().width() << event->size().height();
	    settings->setValue("location/width", event->size().width());
	    settings->setValue("location/height", event->size().height());
	    settings->sync();
	  }
	  QMdiSubWindow::resizeEvent(event); //be sure to pass this event along to the container
	}
	
	void closeEvent(QCloseEvent *event){
	  if( !this->whatsThis().isEmpty() ){
	    //Plugin removed by the user - delete the settings file
	    QFile::remove( settings->fileName() );
	    emit PluginRemoved( this->whatsThis() );
	  }
	  event->accept(); //continue closing the widget
	}
	
};

#endif
