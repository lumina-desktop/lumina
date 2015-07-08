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
	QString PLUGID, prefix;
	QSettings *settings;

public:
	LDPlugin(QWidget *parent = 0, QString id="unknown");
	
	~LDPlugin(){}
	
	QString ID(){
	  return PLUGID;
	}
	
	void setInitialSize(int width, int height);
	
	void saveSetting(QString var, QVariant val){
	  //qDebug() << "Saving Setting:" << prefix+var+QString(" = ")+val.toString();
	  settings->setValue(prefix+var, val);
	  settings->sync();
	}
	
	QVariant readSetting(QString var, QVariant defaultval){
	  return settings->value(prefix+var, defaultval);
	}
	
	void removeSettings(){ //such as when a plugin is deleted
	  QStringList list = settings->allKeys().filter(prefix);
	   for(int i=0; i<list.length(); i++){ settings->remove(list[i]); }
	}
	
	/*virtual void scalePlugin(double xscale, double yscale){
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
	}*/
	
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
