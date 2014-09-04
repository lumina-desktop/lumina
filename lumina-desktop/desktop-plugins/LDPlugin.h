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
//  WARNING: Do *not* setup a custom context menu for plugins!
//     This can prevent access to the general desktop context menu if
//     the plugin was maximized to fill the desktop area!
//===========================================
#ifndef _LUMINA_DESKTOP_DESKTOP_PLUGIN_H
#define _LUMINA_DESKTOP_DESKTOP_PLUGIN_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QDebug>
#include <QSettings>
#include <QMoveEvent>
#include <QResizeEvent>

class LDPlugin : public QWidget{
	Q_OBJECT
	
private:
	QString PLUGID;
	
public:
	QSettings *settings;

	LDPlugin(QWidget *parent = 0, QString id="unknown") : QWidget(parent){
	  PLUGID=id;
	  settings = new QSettings("desktop-plugins",PLUGID);
	}
	
	~LDPlugin(){
	  delete settings;
	}
	
	QString ID(){
	  return PLUGID;
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
};

#endif