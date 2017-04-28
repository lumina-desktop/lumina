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
#include <QSettings>

class TermWindow : public QWidget{
	Q_OBJECT
public:
	TermWindow(QSettings *set);
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
	QSettings *settings;
	QShortcut *hideS, *closeS, *newTabS, *closeTabS, *prevTabS, *nextTabS;
	int screennum;
	bool onTop, CLOSING;
	QPropertyAnimation *ANIM;
	int animRunning; //internal flag for what animation is currently running
	QTimer *activeTimer;

	//Calculate the window geometry necessary based on screen/location
	void CalculateGeom();
	QString GenerateTabID();

private slots:
	//Tab Interactions
	void New_Tab();
	void Close_Tab(int tab = -1);
	void Close_Tab(QString ID); //alternate form of the close routine - based on tab ID
	void Next_Tab();
	void Prev_Tab();
	void focusOnWidget();
	//Animation finishing
	void AnimFinished();
	//Window focus/active status changed
	void activeStatusChanged();

protected:
	void mouseMoveEvent(QMouseEvent*);

signals:
	void TerminalHidden();
	void TerminalVisible();
	void TerminalClosed();
	void TerminalFinished();
};

#endif
