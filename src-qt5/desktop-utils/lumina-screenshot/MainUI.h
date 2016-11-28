//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
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
#include <QSettings>
#include <QAction>
#include <QScreen>

#include <LuminaXDG.h>
#include <LUtils.h>
#include <LuminaX11.h>

#include "ImageEditor.h"

namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

public slots:
	void setupIcons();

private:
	Ui::MainUI *ui;
	bool mousegrabbed;
	QRect lastgeom;
	QString ppath; //previous file path
	WId cwin; //current window to screenshot
	QSettings *settings;
	LXCB *XCB; //Library access to window subsystems
	QTimer *scaleTimer;

	//Image Editor widget
	ImageEditor *IMG;
	QDateTime lastScreenShot;

	void showSaveError(QString path);

private slots:
	//Button Slots
	void closeApplication(){
	  this->close();
	}
	void saveScreenshot();
	void quicksave();

	void startScreenshot();

	void imgselchanged(bool hassel);
	void imgScalingChanged(int percent = -1);
	void sliderChanged();

	//Utility functions to perform a screenshot
	bool getWindow(); //set the "cwin" variable as appropriate
	void getPixmap(); //set the "cpic" variable to the new screenshot

protected:
	void mouseReleaseEvent(QMouseEvent *ev);
	void resizeEvent(QResizeEvent *ev);
};

#endif
