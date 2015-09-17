//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_UTILITIES_TERMINAL_MAIN_WINDOW_H
#define _LUMINA_DESKTOP_UTILITIES_TERMINAL_MAIN_WINDOW_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QTabWidget>
#include <QDir>
#include <QShortcut>
#include <QMouseEvent>

class TermWindow : public QWidget{
	Q_OBJECT
public:
	TermWindow();
	~TermWindow();

	void cleanup(); //called right before the window is closed
	void OpenDirs(QStringList);

	void setCurrentScreen(int num = 0);
	void setTopOfScreen(bool ontop);

public slots:
	void ShowWindow();
	void HideWindow();
	void CloseWindow();
	void ReShowWindow();

private:
	QTabWidget *tabWidget;
	QShortcut *hideS, *closeS, *newTabS, *closeTabS;
	int screennum;
	bool onTop, CLOSING;
	QPropertyAnimation *ANIM;
	int animRunning; //internal flag for what animation is currently running

	//Calculate the window geometry necessary based on screen/location
	void CalculateGeom();
	QString GenerateTabID();

private slots:
	//Tab Interactions
	void New_Tab();
	void Close_Tab(int tab = -1);
	void Close_Tab(QString ID); //alternate form of the close routine - based on tab ID

	//Animation finishing
	void AnimFinished();
	
protected:
	void mouseMoveEvent(QMouseEvent*);

signals:
	void TerminalHidden();
	void TerminalVisible();
	void TerminalClosed();
	void TerminalFinished();
};

#endif
