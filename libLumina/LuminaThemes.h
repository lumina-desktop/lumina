//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class governs all the stylesheet usage and interactions
//  for the Lumina utilities to provide a consistant theme for the system
//===========================================
#ifndef _LUMINA_LIBRARY_THEMES_H
#define _LUMINA_LIBRARY_THEMES_H

#include <QApplication>
#include <QObject>
#include <QFileSystemWatcher>
#include <QString>
#include <QFile>
#include <QDir>
#include <QTimer>

class LTHEME{
public:
  //Read the Themes/Colors/Icons that are available on the system
  static QStringList availableSystemThemes();//returns: [name::::path] for each item
  static QStringList availableLocalThemes();	//returns: [name::::path] for each item
  static QStringList availableSystemColors(); 	//returns: [name::::path] for each item
  static QStringList availableLocalColors(); 	//returns: [name::::path] for each item
  static QStringList availableSystemIcons(); 	//returns: [name] for each item
	
  //Save a new theme/color file
  static bool saveLocalTheme(QString name, QStringList contents);
  static bool saveLocalColors(QString name, QStringList contents);

  //Return the currently selected Theme/Colors/Icons
  static QStringList currentSettings(); //returns [theme path, colorspath, iconsname, font, fontsize]

  //Change the current Theme/Colors/Icons
  static bool setCurrentSettings(QString themepath, QString colorpath, QString iconname, QString font, QString fontsize);
	
  //Return the complete stylesheet for a given theme/colors
  static QString assembleStyleSheet(QString themepath, QString colorpath, QString font, QString fontsize);
  
};


//Simple class to setup a utility to use the Lumina theme
//-----Example usage in "main.cpp" -------------------------------
// QApplication a(argc,argv);
// LuminaThemeEngine themes(&a)
//------------------------------------------------------------------------------------
// Note: If you also use LuminaXDG::findIcons() in the application and you want
// to dynamically update those icons - connect to the updateIcons() signal
//-------------------------------------------------------------------------------------
// QMainWindow w; //(or whatever the main app window is)
//  QObject::connect(themes,SIGNAL(updateIcons()), &w, SLOT(updateIcons()) );
//------------------------------------------------------------------------------------
class LuminaThemeEngine : public QObject{
	Q_OBJECT
public:
	LuminaThemeEngine(QApplication *app);
	~LuminaThemeEngine();

private:
	QApplication *application;
	QFileSystemWatcher *watcher;
	QString theme,colors,icons, font, fontsize; //current settings
	QTimer *syncTimer;

private slots:
	void watcherChange();
	void reloadFiles();

signals:
	void updateIcons();
};


#endif
