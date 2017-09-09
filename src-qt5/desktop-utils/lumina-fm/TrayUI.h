//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the system tray icon for queueing/running file operations
//===========================================
#ifndef _LUMINA_FILE_MANAGER_FILE_OP_SYSTRAY_H
#define _LUMINA_FILE_MANAGER_FILE_OP_SYSTRAY_H

#include "OPWidget.h"

#include <QSystemTrayIcon>
#include <QMenu>

class TrayUI : public QSystemTrayIcon{
	Q_OBJECT
public:
	enum FILEOP{MOVE, COPY, DELETE}; //File Operations

	TrayUI(QObject *parent = 0);
	~TrayUI();

public slots:
	void StartOperation( FILEOP op, QStringList oldF, QStringList newF);

private:
	QList<OPWidget*> OPS;

	void createOP( FILEOP, QStringList oldF, QStringList newF);

private slots:
	void TrayActivated();

	//Operation Widget Responses
	void OperationClosed(QString ID);
	void OperationStarted(QString ID);
	void OperationFinished(QString ID);

	void checkJobs(); //see if any jobs are still active/visible, otherwise hide the tray icon

signals:
	void JobsFinished();

};
#endif
