//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_WINDOW_FRAME_H
#define _LUMINA_DESKTOP_WINDOW_FRAME_H

#include "GlobalDefines.h"

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
	enum ModState{Normal, Move, ResizeTop, ResizeTopRight, ResizeRight, ResizeBottomRight, ResizeBottom, ResizeBottomLeft, ResizeLeft, ResizeTopLeft};
	ModState activeState;
	QPoint offset; //needed for movement calculations (offset from mouse click to movement point)
	//Functions for getting/setting state
	ModState getStateAtPoint(QPoint pt, bool setoffset = false); //generally used for mouse location detection
	void setMouseCursor(ModState, bool override = false);  //Update the mouse cursor based on state
	
	//General Properties/Modifications
	WId CID; //Client ID
	QWindow *WIN; //Embedded window container
	//QBackingStore *WINBACK;
	void SyncSize(); //sync the window/frame geometries
	void SyncText();
	
	//Window Frame Widgets/Items
	QLabel *titleBar, *title, *icon;
	QToolButton *minB, *maxB, *closeB, *otherB;
	QMenu *otherM; //menu of "other" actions for the window
	QRect normalGeom; //used for restoring back to original size after maximization/fullscreen
	
	//Animations
	QPropertyAnimation *anim; //used for appear/disappear animations
	QRect lastGeom; //used for appear/disappear animations
	void showAnimation(LWM::WindowAction);
	
public slots:
	//These slots are generally used for the outside event watcher to prod for changes
	void updateAppearance(); //reload the theme and change styling as necessary
	void windowChanged(LWM::WindowAction);

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