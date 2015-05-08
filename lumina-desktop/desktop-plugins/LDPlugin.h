//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class is the generic container layout for all desktop plugins
//  Simply subclass this when creating a new plugin to enable correct
//    visibility and usage within the desktop window
//===========================================
//  WARNING: Do *not* setup a custom context menu for the entire plugins area!
//     This can prevent access to the general desktop context menu if
//     the plugin was maximized to fill the desktop area!
//===========================================
#ifndef _LUMINA_DESKTOP_DESKTOP_PLUGIN_H
#define _LUMINA_DESKTOP_DESKTOP_PLUGIN_H

#include <QObject>
#include <QFrame>
#include <QWidget>
#include <QString>
#include <QDebug>
#include <QSettings>
#include <QMoveEvent>
#include <QResizeEvent>

class LDPlugin : public QFrame{
	Q_OBJECT
	
private:
	QString PLUGID;
	
public:
	QSettings *settings;

	LDPlugin(QWidget *parent = 0, QString id="unknown", bool opaque = false) : QFrame(parent){
	  PLUGID=id;
	  settings = new QSettings("desktop-plugins",PLUGID);
	  //Use two values for stylesheet access, Visible or normal plugin base
	  if(opaque){ this->setObjectName("LuminaDesktopPluginVisible"); }
	  else{ this->setObjectName("LuminaDesktopPlugin"); }
	  //Use plugin-specific values for stylesheet control (applauncher, desktopview, etc...)
	  qDebug() << "set Objectname:" << id.section("---",0,0).section("::",0,0);
	  this->setObjectName(id.section("---",0,0).section("::",0,0));
	}
	
	~LDPlugin(){
	  
	}
	
	QString ID(){
	  return PLUGID;
	}
	
	void setInitialSize(int width, int height){
	    //Note: Only run this in the plugin initization routine:
	    //  if the plugin is completely new (first time used), it will be this size
	    if(settings->allKeys().isEmpty()){
		//Brand new plugin: set initial size
		settings->setValue("location/width",width);
		settings->setValue("location/height",height);
		settings->sync();
	    }
	}

	virtual void scalePlugin(double xscale, double yscale){
          //This can be re-implemented in the subclassed plugin as necessary
	  // Example: If there are icons in the plugin which should also be re-scaled

	  int val = settings->value("location/width",0).toInt();
	  if(val>0){ val = qRound(val*xscale); }
	  settings->setValue("location/width",val);

	  val = settings->value("location/height",0).toInt();
	  if(val>0){ val = qRound(val*yscale); }
	  settings->setValue("location/height",val);

  	  val = settings->value("location/x",0).toInt();
	  if(val>0){ val = qRound(val*xscale); }
	  settings->setValue("location/x",val);

  	  val = settings->value("location/y",0).toInt();
	  if(val>0){ val = qRound(val*yscale); }
	  settings->setValue("location/y",val);
	}
	
public slots:
	virtual void LocaleChange(){
	  //This needs to be re-implemented in the subclassed plugin
	    //This is where all text is set/translated
	}
	virtual void ThemeChange(){
	  //This needs to be re-implemented in the subclassed plugin
	    //This is where all the visuals are set if using Theme-dependant icons.
	}
	
signals:
	void OpenDesktopMenu();
	
};

#endif
