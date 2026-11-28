//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_SEARCH_DIALOG_WORKER_H
#define _LUMINA_SEARCH_DIALOG_WORKER_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QMutex>
#include <atomic>


class Worker : public QObject{
	Q_OBJECT
public:
	Worker(QObject *parent = 0);
	~Worker();

	//The search directories are set from the GUI thread while a search may be running in the worker
	// thread: access them only through these (a running search uses its own snapshot)
	void setStartDir(QString dir);
	QString startDir() const;
	void setSkipDirs(QStringList dirs);
	QStringList skipDirs() const;

public slots:
	void StartSearch(QString term, bool isApp);
	void StopSearch();

private:
	QStringList applist;
	bool appsLoaded; //applist is filled on the first app search, in the worker thread
	void loadApps();
	std::atomic_bool stopsearch; //set from the GUI thread (StopSearch) while the worker thread searches
	mutable QMutex dirMutex;
	QString m_startDir;
	QStringList m_skipDirs;
	QStringList curSkipDirs; //snapshot for the running search
	QString sterm;
	bool sapp;
	bool searchDir(QString);

private slots:
	void beginsearch();
	
signals:
	void FoundItem(QString path);
	void SearchUpdate(QString);
	void SearchDone();
};

#endif
