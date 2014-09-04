//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_SCREENSHOT_DIALOG_H
#define _LUMINA_SCREENSHOT_DIALOG_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QPixmap>
#include <QTimer>
#include <QDesktopWidget>
#include <QDir>
#include <QInputDialog>

#include <LuminaXDG.h>
#include <LuminaUtils.h>

namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

private:
	Ui::MainUI *ui;
	QPixmap cpic; //current picture
	QString ppath; //previous file path
	WId cwin; //current window to screenshot

private slots:
	//Button Slots
	void closeApplication(){
	  this->close();
	}
	void saveScreenshot();
	void startScreenshot();

	//Utility functions to perform a screenshot
	bool getWindow(); //set the "cwin" variable as appropriate
	void getPixmap(); //set the "cpic" variable to the new screenshot
};

#endif