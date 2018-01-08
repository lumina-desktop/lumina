//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is a simple class for managing all the various desktop
//  screensaver plugins that could be available
//===========================================
//  NOTE:
//  This class has a heirarchy-based lookup system
//  USER plugins > SYSTEM plugins
//  XDG_DATA_HOME/lumina-desktop/screensavers >  XDG_DATA_DIRS/lumina-desktop/screensavers
//===========================================
#ifndef _LUMINA_DESKTOP_BASE_PLUGINS_CLASS_H
#define _LUMINA_DESKTOP_BASE_PLUGINS_CLASS_H

#include <QJsonObject>
#include <QString>
#include <QUrl>
#include <QObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QDebug>

class BasePlugin{
protected:
	QString currentfile;

public:
	BasePlugin();
	virtual ~BasePlugin();

	virtual void loadFile(QString path);
	bool isLoaded() { return !data.isEmpty(); };
	bool containsDefault(QString obj) { return data.value(obj).toObject().contains("default"); }

  /**
   * Check if the plugin is valid as long as the JSON is not empty,
   * it contains at least a "name", "qml", and "description" object,
   * and the "name" and "description" objects contain a "default" key.
  **/
	virtual bool isValid() = 0;

	virtual QString translatedObject(QString obj);
	virtual QUrl scriptURL();

	QJsonObject data; //Hazardous to manually modify
	QString relDir;
};

class PluginSystem{
public:
  template <class T>
  static T findPlugin(QString, QString);

  template <class T>
  static QList<T> findAllPlugins(QString, bool validonly=true);
};

// ===================
// 	Base PLUGIN SYSTEM
// ===================
template <class T> 
T PluginSystem::findPlugin(QString name, QString REL_DIR){
  //qDebug() << "FindPlugin:" << name;
  T BaseP;
  if(name.startsWith("/") && QFile::exists(name)){ BaseP.loadFile(name); return BaseP;} //absolute path give - just load that one
  //Cleanup the input name and ensure it has the right suffix
  name = name.section("/",-1);
  if(!name.endsWith(".json")){ name.append(".json"); }
  //Get the list of directories to search
  QStringList dirs;
  dirs << QString(getenv("XDG_DATA_HOME")) << QString(getenv("XDG_DATA_DIRS")).split(":");
  //Look for that file within these directories and return the first one found
  for(int i=0; i<dirs.length(); i++){
    if(!QFile::exists(dirs[i]+REL_DIR+"/"+name)){ continue; }
    BaseP.loadFile(dirs[i]+REL_DIR+"/"+name);
    if(BaseP.isValid()){ break; } //got a good one - stop here
  }
  return BaseP;
}

template <class T>
QList<T> PluginSystem::findAllPlugins(QString REL_DIR, bool validonly) {
  QList<T> LIST;
  //Get the list of directories to search
  QStringList dirs;
  dirs << QString(getenv("XDG_DATA_HOME")) << QString(getenv("XDG_DATA_DIRS")).split(":");
 //Look for that file within these directories and return the first one found
  for(int i=0; i<dirs.length(); i++){
    if(!QFile::exists(dirs[i]+REL_DIR)){ continue; }
    QDir dir(dirs[i]+REL_DIR);
    QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files, QDir::Name);
     //qDebug() << "Found Files:" << files;
    for(int j=0; j<files.length(); j++){
      T tmp;
      tmp.loadFile(dir.absoluteFilePath(files[j]));
      //qDebug() << "Loaded File:" << files[j] << tmp.isValid();
      if(!validonly || tmp.isValid()){ LIST << tmp; }
    }
  }
  return LIST;
}
#endif
