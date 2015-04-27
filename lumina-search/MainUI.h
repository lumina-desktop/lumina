//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_SEARCH_DIALOG_H
#define _LUMINA_SEARCH_DIALOG_H

#include <QMainWindow>
#include <QString>
#include <QTimer>
#include <QSettings>
#include <QThread>
#include <QProcess>
#include <QListWidgetItem>

#include <LuminaXDG.h>

#include "Worker.h"

namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

	void disableExcludes();
	void setSearchDirectory(QString path);
	void setSearchTerm(QString text);

public slots:
	void setupIcons();
	void startSearch(); //emit the proper signal for the worker

private:
	Ui::MainUI *ui;
	QThread *workthread;
	QTimer *livetime;
	Worker *searcher;
	QSettings *settings;

	void updateDefaultStatusTip();

private slots:
	//Button Slots
	void closeApplication(){
	  this->close();
	}
	void LaunchItem();
	void LaunchItem(QListWidgetItem*);
	void searchTypeChanged();
	void configureSearch();
	
	void searchChanged(); //for active searching while typing
	
	//Worker Interaction
	//void startSearch(); //Moved to a public slot
	void foundSearchItem(QString path); //To get the worker's results
	void stopSearch();
	void searchMessage(QString);
	void searchFinished();
	
signals:
	void SearchTerm(QString term, bool isApp); //to connect to the worker's slot
};

#endif
