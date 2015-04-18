//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the background class for running long-lived calculations
//===========================================
#ifndef _LUMINA_FILE_MANAGER_BACKGROUND_WORKER_H
#define _LUMINA_FILE_MANAGER_BACKGROUND_WORKER_H

#include <QObject>
#include <QStringList>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

#include <LuminaOS.h>


class BackgroundWorker : public QObject{
	Q_OBJECT
public:
	BackgroundWorker();
	~BackgroundWorker();
		
private:
	QStringList multiFilter, imgFilter;
	QString cdir, csnapdir; //last directory checked (and base snapshot dir found)
	QString ItemsInstatusBar(QFileInfoList, QString);

public slots:
	//Kickoff processes with these slots
        // and then listen for the appropriate signals when finished
	void startDirChecks(QString path);
	void createStatusBarMsg(QFileInfoList fileList, QString path, QString messageFolders, QString messageFiles);

signals:
	void ImagesAvailable(QStringList files);
	void MultimediaAvailable(QStringList files);
	void SnapshotsAvailable(QString basedir, QStringList snappaths);
	void Si_DisplayStatusBar(QString);
	
};

#endif
