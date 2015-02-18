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


class Worker : public QObject{
	Q_OBJECT
public:
	Worker(QObject *parent = 0);
	~Worker();

	QString startDir;
	QStringList skipDirs;

public slots:
	void StartSearch(QString term, bool isApp);
	void StopSearch();

private:
	QStringList applist;
	bool stopsearch;
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