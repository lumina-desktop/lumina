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

class Browser : public QObject{
	Q_OBJECT
public:
	Browser(QObject *parent = 0);
	~Browser();

	void showHiddenFiles(bool);
	bool showingHiddenFiles();

private:
	QString currentDir;
	QFileSystemWatcher *watcher;
	bool showHidden;

	void loadItem(QFileInfo info); //this is the main loader class - multiple instances each run in a separate thread

private slots:
	void fileChanged(QString); //tied into the watcher - for file change notifications
	void dirChanged(QString); // tied into the watcher - for new/removed files in the current dir

public slots:
	QString loadDirectory(QString dir = "");

signals:
	//Main Signals
	void itemUpdated(QString item); //emitted if a file changes after the initial scan
	void clearItems(); //emitted when dirs change for example
	void itemDataAvailable(QIcon, LFileInfo);

	//Start/Stop signals for loading of data
	void itemsLoading(int); //number of items which are getting loaded
	void itemsDoneLoading(); //emitted when all items are done loading

};

#endif
