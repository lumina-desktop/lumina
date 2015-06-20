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
#include <QIcon>
#include <QTimer>

#include "LDPlugin.h"

class LDPluginContainer : public QMdiSubWindow{
	Q_OBJECT
	
private:
	QTimer *syncTimer;
	bool locked, setup;
	LDPlugin *PLUG;

private slots:
	void saveGeometry(){
	    if(PLUG==0){ return; }
	    PLUG->saveSetting("location/x", this->pos().x());
	    PLUG->saveSetting("location/y", this->pos().y());
	    PLUG->saveSetting("location/width", this->size().width());
	    PLUG->saveSetting("location/height", this->size().height());
	}
	
public:
	LDPluginContainer(LDPlugin *plugin = 0, bool islocked = true) : QMdiSubWindow(){
	  locked = islocked;
	  setup=true;
	  PLUG = plugin;
	  syncTimer = new QTimer(this);
	    syncTimer->setInterval(500); //save settings 1 second after it is moved
	    syncTimer->setSingleShot(true); //no repeats
	    connect(syncTimer, SIGNAL(timeout()), this, SLOT(saveGeometry()) );
	  this->setWhatsThis(plugin->ID());
	  this->setContentsMargins(0,0,0,0);
	  if(!locked){
	    this->setStyleSheet("LDPluginContainer{ border-size: 1px;}");
	    this->setWindowTitle( plugin->ID().replace("---"," - ") );
	    //this->setWidget( new QWidget() );
	    this->setWidget( plugin );
	    //this->setWindowIcon(QIcon()); //remove the Qt icon
	  }else{
	    this->setStyleSheet("LDPluginContainer{ background: transparent; border: none;}");
	    this->setWidget(plugin);
	  }
	  //qDebug() << "New Container:" << PLUG->size() << PLUG->sizeHint();
	}
	
	~LDPluginContainer(){
	}

	void loadInitialPosition(){
	  QRect set(PLUG->readSetting("location/x",-12345).toInt(), PLUG->readSetting("location/y",-12345).toInt(), PLUG->readSetting("location/width",this->widget()->size().width()).toInt(), PLUG->readSetting("location/height",this->widget()->size().height()).toInt());
	  //qDebug() << "Initial Plugin Location:" << set.x() << set.y() << set.width() << set.height();
	    if(set.height() < 10){ set.setHeight(10); } //to prevent foot-shooting
	    if(set.width() < 10){ set.setWidth(10); } //to prevent foot-shooting
	    if(set.x()!=-12345 && set.y()!=-12345){
	      //custom location specified
	      //qDebug() << " - Found Geom:" << set;
	      this->setGeometry(set);
	      //this->move(set.x(), set.y());
	    }else{
	      //qDebug() << " - Found Size:" << set;
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
	    if(syncTimer->isActive()){ syncTimer->stop(); }
	    syncTimer->start();
	    //qDebug() << "DP Move:" << event->pos().x() << event->pos().y();
	  }
	  QMdiSubWindow::moveEvent(event); //be sure to pass this event along to the container
	}
	
	void resizeEvent(QResizeEvent *event){
	  //Save this size info to the settings
	  if(!locked && !setup){
	    //qDebug() << "DP Resize:" << event->size().width() << event->size().height();
	    if(syncTimer->isActive()){ syncTimer->stop(); }
	    syncTimer->start();
	  }
	  QMdiSubWindow::resizeEvent(event); //be sure to pass this event along to the container
	}
	
	void closeEvent(QCloseEvent *event){
	  //qDebug() << "Desktop Plugin Close Event:" << this->whatsThis();
	  if( !this->whatsThis().isEmpty() && !locked){
	    //Plugin removed by the user - delete the settings file
	    locked = true; //ensure that the save settings routines don't do anything during the close
	    emit PluginRemoved( this->whatsThis() );
	  }
	  if(syncTimer->isActive()){ syncTimer->stop(); } //prevent save routine from running in a moment
	  //settings = 0; //ensure we don't touch the settings file after a close event
	  QMdiSubWindow::closeEvent(event); //continue closing this window
	}
	
};

#endif
