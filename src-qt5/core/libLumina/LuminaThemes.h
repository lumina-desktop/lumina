//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
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
#include <QDateTime>
#include <QStyle>
#include <QProxyStyle>

class LTHEME{
public:
  //Read the Themes/Colors/Icons that are available on the system
  static QStringList availableSystemThemes();//returns: [name::::path] for each item
  static QStringList availableSystemStyles();//returns: [name::::path] for each item
  static QStringList availableLocalThemes();	//returns: [name::::path] for each item
  static QStringList availableLocalStyles();	//returns: [name::::path] for each item
  static QStringList availableSystemColors(); 	//returns: [name::::path] for each item
  static QStringList availableLocalColors(); 	//returns: [name::::path] for each item
  static QStringList availableSystemIcons(); 	//returns: [name] for each item
  static QStringList availableSystemCursors();	//returns: [name] for each item

  //Save a new theme/color file
  static bool saveLocalTheme(QString name, QStringList contents);
  static bool saveLocalColors(QString name, QStringList contents);

  //Return the currently selected Theme/Colors/Icons
  static QStringList currentSettings(); //returns [theme path, colorspath, iconsname, font, fontsize]
  static QString currentCursor(); //returns: current cursor theme name

  //Change the current Theme/Colors/Icons
  static bool setCurrentSettings(QString themepath, QString colorpath, QString iconname, QString font, QString fontsize);
  static bool setCursorTheme(QString cursorname);
  static bool setCurrentStyles(QStringList paths); //ordered by priority: lowest -> highest

  //Return the complete stylesheet for a given theme/colors
  static QString assembleStyleSheet(QString themepath, QString colorpath, QString font, QString fontsize);

  //Additional info for a cursor theme
  static QStringList cursorInformation(QString name); //returns: [Name, Comment, Sample Image File]

  //Environment settings
  static QStringList CustomEnvSettings(bool useronly = false); //view all the key=value settings
  static void LoadCustomEnvSettings(); //will push the custom settings into the environment (recommended before loading the initial QApplication)
  static bool setCustomEnvSetting(QString var, QString val); //variable/value pair (use an empty val to clear it)
  static QString readCustomEnvSetting(QString var);

};

// Qt Style override to allow custom themeing/colors
/*class LuminaThemeStyle : public QProxyStyle{
	Q_OBJECT
private:
	bool darkfont;

public:
	LuminaThemeStyle();
	~LuminaThemeStyle();

	//Function to update the style (for use by the theme engine)
	void update();	
	//Subclassed functions 
	void drawItemText(QPainter*, const QRect&, int, const QPalette&, bool, const QString&, QPalette::ColorRole) const;

};*/

//Simple class to setup a utility to use the Lumina theme
//-----Example usage in "main.cpp" -------------------------------
// LTHEME::LoadCustomEnvSettings();
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

	void refresh();

private:
	QApplication *application;
	QFileSystemWatcher *watcher;
	QString theme,colors,icons, font, fontsize, cursors; //current settings
	QTimer *syncTimer;
	QDateTime lastcheck;
	//LuminaThemeStyle *style;

private slots:
	void watcherChange(QString);
	void reloadFiles();

signals:
	void updateIcons(); 		//Icon theme changed
	void updateCursors(); 	//Cursor theme changed
	void EnvChanged(); 		//Some environment variable(s) changed
};	

#endif
