//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is a simple class for loading/serving icon files
// from the icon theme or local filesystem
//===========================================
#ifndef _LUMINA_LIBRARY_ICON_CACHE_H
#define _LUMINA_LIBRARY_ICON_CACHE_H

#include <QHash>
#include <QIcon>
#include <QPixmap>
#include <QFileSystemWatcher>
#include <QString>
#include <QFile>
#include <QDateTime>
#include <QAbstractButton>
#include <QLabel>
#include <QAction>
#include <QPointer>

//Data structure for saving the icon/information internally
struct icon_data{
  QString fullpath;
  QDateTime lastread;
  QList<QPointer<QLabel> > pendingLabels;
  QList<QPointer<QAbstractButton> > pendingButtons;
  QList<QPointer<QAction> > pendingActions;
  QList<QPointer<QMenu> > pendingMenus;
  QIcon icon;
  QIcon thumbnail;
};

class LIconCache : public QObject{
	Q_OBJECT
public:
	LIconCache(QObject *parent = 0);
	~LIconCache();

	//Static method for using this class (DO NOT MIX WITH GLOBAL OBJECT METHOD)
	// Either use this the entire time, or use a saved/global object - pick one and stick with it
	//  otherwise you may end up with multiple icon cache's running for your application
	static LIconCache* instance();

	//Icon Checks
	bool exists(QString icon);
	bool isLoaded(QString icon);
	QString findFile(QString icon); //find the full path of a given file/name (searching the current Icon theme)

	//Special loading routines for QLabel and QAbstractButton (pushbutton, toolbutton, etc)
	void loadIcon(QAbstractButton *button, QString icon, bool noThumb = false);
	void loadIcon(QLabel *label, QString icon, bool noThumb = false);
	void loadIcon(QAction *action, QString icon, bool noThumb = false);
	void loadIcon(QMenu *action, QString icon, bool noThumb = false);

	QIcon loadIcon(QString icon, bool noThumb = false); //generic loading routine - does not background the loading of icons when not in the cache

	void clearIconTheme(); //use when the icon theme changes to refresh all requested icons
	void clearAll(); //Clear all cached icons

private:
	QHash<QString, icon_data> HASH;
	QFileSystemWatcher *WATCHER;

	icon_data createData(QString icon);
	QStringList getChildIconDirs(QString path); //recursive function to find directories with icons in them
	QStringList getIconThemeDepChain(QString theme, QStringList paths);

	void startReadFile(QString id, QString path);
	void ReadFile(LIconCache *obj, QString id, QString path);

	bool isThemeIcon(QString id);
	QIcon iconFromTheme(QString id);

private slots:
	void IconLoaded(QString id, QDateTime sync, QByteArray *data);

signals:
	void InternalIconLoaded(QString, QDateTime, QByteArray*); //INTERNAL SIGNAL - DO NOT USE in other classes/objects
	void IconAvailable(QString); //way for classes to listen/reload icons as they change
};

#endif
