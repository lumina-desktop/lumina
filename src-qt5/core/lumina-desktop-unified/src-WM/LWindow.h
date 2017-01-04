//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_WINDOW_FRAME_H
#define _LUMINA_DESKTOP_WINDOW_FRAME_H

#include "GlobalDefines.h"

class LWindowFrame : public QFrame{
	Q_OBJECT
public:
	LWindowFrame(WId client, QWidget *parent = 0); //MUST have a valid client window
	~LWindowFrame();

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
	QWidget *WinWidget;
	bool Closing;
	LWM::WindowAction lastAction;
	//QBackingStore *WINBACK;
	void SyncSize(bool fromwin = false); //sync the window/frame geometries
	void SyncText();
	
	//Window Frame Widgets/Items
	QLabel *titleBar, *title, *icon;
	QToolButton *minB, *maxB, *closeB, *otherB;
	QMenu *otherM; //menu of "other" actions for the window
	QRect normalGeom; //used for restoring back to original size after maximization/fullscreen
	
	//Animations
	QPropertyAnimation *anim; //used for appear/disappear animations
	QRect lastGeom; //used for appear/disappear animations
	void showAnimation(LWM::WindowAction); //sets lastAction
	void ShowClient(bool show);
	
public slots:
	//These slots are generally used for the outside event watcher to prod for changes
	void updateAppearance(); //reload the theme and change styling as necessary
	void windowChanged(LWM::WindowAction);

private slots:
	void finishedAnimation(); //uses lastAction
	void closeClicked();
	void minClicked();
	void maxClicked();
	void otherClicked(QAction*);

	void CloseAll();
	
protected:
	void mousePressEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);

signals:
	void Finished(); //This means the window is completely finished (with animations and such) and should be removed from any lists
	
};

class LWindow : public QObject{
	Q_OBJECT
signals:
	void Finished(WId client); //ready to be removed
private:
	WId CID;
	LWindowFrame *FID;
	bool needsFrame(QList<LXCB::WINDOWTYPE> list){
	  if(list.isEmpty()){ return !LWM::SYSTEM->WM_ICCCM_GetClass(CID).contains("Lumina-DE"); } //assume a normal window (fallback)
	  return !(list.contains(LXCB::T_DESKTOP) || list.contains(LXCB::T_DOCK) || list.contains(LXCB::T_TOOLBAR) \
	        || list.contains(LXCB::T_SPLASH) || list.contains(LXCB::T_DROPDOWN_MENU) \
		|| list.contains(LXCB::T_TOOLTIP) || list.contains(LXCB::T_POPUP_MENU) || list.contains(LXCB::T_TOOLTIP) \
	        || list.contains(LXCB::T_COMBO) || list.contains(LXCB::T_DND) );
	}
private slots:
	void frameclosed(){
	  qDebug() << " - Window got frame closed signal";
	  //FID->close();
	  //delete FID;
	  emit Finished(CID);
	}
public:
	LWindow(WId client){
	  FID= 0;
	  CID = client;
	  if( needsFrame(LWM::SYSTEM->WM_Get_Window_Type(CID)) ){
	    FID = new LWindowFrame(CID);
	    connect(FID, SIGNAL(Finished()), this, SLOT(frameclosed()) );
	  }
	}
	~LWindow(){ 
	  if(FID!=0){delete FID;}
	}
	
	WId clientID(){ return CID; }
	bool hasFrame(){ return FID!=0; }
	LWindowFrame* frame(){ return FID; }

};
#endif
