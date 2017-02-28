//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_CONFIG_MAIN_WINDOW_H
#define _LUMINA_CONFIG_MAIN_WINDOW_H
#include "globals.h"
//#include "pages/getPage.h"

namespace Ui{
	class mainWindow;
};

class mainWindow : public QMainWindow{
	Q_OBJECT
public:
	mainWindow();
	~mainWindow();

public slots:
	void slotSingleInstance(QStringList args);
	void setupIcons();
	void loadMonitors();

private:
	Ui::mainWindow *ui;
	QShortcut *backShortcut, *quitShortcut;
	QString cpage; //current page


	void changePage(QString id);

private slots:
	//Page signals
	void pageCanSave(bool);
	void pageSetTitle(QString);
	bool page_change(QString);
	//Toolbar actions
	void on_actionSave_triggered();
	void on_actionBack_triggered();
        void changeMonitor(QAction*);
	void on_quitShortcut_Triggered();
};
#endif
