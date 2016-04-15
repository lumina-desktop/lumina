//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_SCREEN_CONFIG_DIALOG_H
#define _LUMINA_SCREEN_CONFIG_DIALOG_H

#include <QMainWindow>
#include <QRect>
#include <QString>
#include <QList>

class ScreenInfo{
  public:	
	QString ID;
	QRect geom; //screen geometry
	int order; //left to right
	QStringList resList;
	ScreenInfo(){
	  order = -1; //initial value is invalid	
	}
	~ScreenInfo(){}
};

namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

public slots:
	void slotSingleInstance(){
	  this->raise();
	  this->show();
	}
	void loadIcons();

private:
	Ui::MainUI *ui;
	QList<ScreenInfo> SCREENS;
	ScreenInfo currentScreenInfo();

	QStringList currentOpts();

private slots:
	void UpdateScreens();
	void ScreenSelected();
	void MoveScreenLeft();
	void MoveScreenRight();
	void DeactivateScreen(QString device = "");
	void ActivateScreen();
	void ApplyChanges(); //config changes
};

#endif
