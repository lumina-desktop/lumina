//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the system tray icon for queueing/running file operations
//===========================================
#ifndef _LUMINA_FILE_MANAGER_FILE_OP_OPWIDGET_H
#define _LUMINA_FILE_MANAGER_FILE_OP_OPWIDGET_H

#include "FODialog.h"
#include "ScrollDialog.h"
namespace Ui{
	class OPWidget;
};

class OPWidget : public QWidget{
	Q_OBJECT
public:
	OPWidget(QWidget *parent = 0);
	~OPWidget();

	QWidgetAction* widgetAction(); //for loading the widget into a menu

	void setupOperation(QString optype, QStringList oldF, QStringList newF);

	bool isDone();

	//Status reporting after worker finishes
	bool hasErrors(); 
	float duration(); //in seconds
	QString finalStat(); //Final status message

public slots:
	void startOperation();

private:
	Ui::OPWidget *ui;
	ScrollDialog *dlg;
	//Main Objects
	QWidgetAction *WA;
	FOWorker *worker;
	QThread *workthread;
	//Bookkeeping items for statistics and such
	qint64 starttime, endtime;  //in ms
	QStringList Errors;
	QString tract; //translated action

private slots:
	void closeWidget();
	void showErrors();
	void opFinished(QStringList); //errors
	void opUpdate(int, int, QString, QString); //current, total, old file, new file

signals:
	void starting(QString);
	void finished(QString);
	void closed(QString);
};
#endif
