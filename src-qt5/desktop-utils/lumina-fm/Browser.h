//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the main browsing backend for the file manager
//===========================================
#ifndef _LUMINA_FM_BROWSE_BACKEND_H
#define _LUMINA_FM_BROWSE_BACKEND_H

#include <QObject>
#include <QString>
#include <QFileSystemWatcher>
#include <QIcon>
//#include <QFutureWatcher>

#include <LuminaXDG.h>
/*class FileItem{
public:
	QString name;
	QByteArray icon;

	FileItem(){}
	~FileItem(){};
};*/

class Browser : public QObject{
	Q_OBJECT
public:
	Browser(QObject *parent = 0);
	~Browser();

	QString currentDirectory();
	void showHiddenFiles(bool);
	bool showingHiddenFiles();

	//FileItem loadItem(QString info); //this is the main loader class - multiple instances each run in a separate thread

private:
	QString currentDir;
	QFileSystemWatcher *watcher;
	bool showHidden;
	QStringList imageFormats, oldFiles;
	QHash<QString, QIcon> mimeIcons; //cache for quickly re-using QIcons
	

	void loadItem(QString info); //this is the main loader class - multiple instances each run in a separate thread
	QIcon loadIcon(QString icon); //simplification for using/populating the mimIcons cache

private slots:
	void fileChanged(QString); //tied into the watcher - for file change notifications
	void dirChanged(QString); // tied into the watcher - for new/removed files in the current dir

	void futureFinished(QString, QByteArray);

public slots:
	void loadDirectory(QString dir = "");

signals:
	//Main Signals
	void itemRemoved(QString item); //emitted if a file was removed from the underlying
	void clearItems(); //emitted when dirs change for example
	void itemDataAvailable(QIcon, LFileInfo);

	//Start/Stop signals for loading of data
	void itemsLoading(int); //number of items which are getting loaded

	//Internal signal for the alternate threads
	void threadDone(QString, QByteArray);
};

#endif
