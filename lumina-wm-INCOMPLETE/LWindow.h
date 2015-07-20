//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_SCREEN_SAVER_H
#define _LUMINA_DESKTOP_SCREEN_SAVER_H

#include <QFrame>
#include <QLabel>
#include <QToolButton>
#include <QMenu>
#include <QMouseEvent>
#include <QAction>
#include <QPoint>

class LWindow : public QFrame{
	Q_OBJECT
public:
	LWindow(WId client); //MUST have a valid client window
	~LWindow();

	WId clientID();
	bool hasFrame();

private:
	void InitWindow(); //Initialize all  the internal widgets

	//Window status 
	enum ModStatus{Normal, Move, ResizeTop, ResizeTopRight, ResizeRight, ResizeBottomRight, ResizeBottom, ResizeBottomLeft, ResizeLeft, ResizeTopLeft};
	ModStatus activeState;
	//Functions for getting/setting state
	ModStatus getStateAtPoint(QPoint); //generally used for mouse location detection
	void setMouseCursor(ModStatus);  //Update the mouse cursor based on state
	
	//General Properties
	WId CID; //Client ID
	
	//Window Frame Widgets/Items
	QLabel *titleBar, *title, *icon;
	QToolButton *minB, *maxB, *closeB, *otherB;
	QMenu *otherM; //menu of "other" actions for the window

public slots:
	//These slots are generally used for the outside event watcher to 
	void updateAppearance(); //reload the theme and change styling as necessary
	void propertiesChanged();
	void 

private slots:
	void closeClicked();
	void minClicked();
	void maxClicked();
	void otherClicked(QAction*);

protected:
	void mousePressEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);

signals:


};

#endif