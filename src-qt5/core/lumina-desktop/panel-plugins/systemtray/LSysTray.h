//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_SYSTRAY_H
#define _LUMINA_DESKTOP_SYSTRAY_H

//Qt includes
#include <QFrame>
#include <QHBoxLayout>
#include <QDebug>
#include <QX11Info>
#include <QCoreApplication>

//Local includes
#include "../LPPlugin.h"
#include "TrayIcon.h"

//SYSTEM TRAY STANDARD DEFINITIONS
#define SYSTEM_TRAY_REQUEST_DOCK 0
#define SYSTEM_TRAY_BEGIN_MESSAGE 1
#define SYSTEM_TRAY_CANCEL_MESSAGE 2

class LSysTray : public LPPlugin{
	Q_OBJECT
public:
	LSysTray(QWidget *parent = 0, QString id="systemtray", bool horizontal=true);
	~LSysTray();

	virtual void AboutToClose(){
	  this->stop();		
	}
	
private:
	bool isRunning, stopping, checking, pending;
	QList<TrayIcon*> trayIcons;
	QFrame *frame;
	QBoxLayout *LI; //layout items
	QTimer *upTimer; //manual timer to force refresh of all items
	
private slots:
	void checkAll();
	void UpdateTrayWindow(WId win);

	//void removeTrayIcon(WId win);

public slots:
	void start();
	void stop();

	virtual void OrientationChange(){
	   //make sure the internal layout has the same orientation as the main widget
	   LI->setDirection( this->layout()->direction() );
	   //Re-adjust the maximum widget size
	  int sz;
	  if(this->layout()->direction()==QBoxLayout::LeftToRight){
	    this->setMaximumSize( trayIcons.length()*this->height(), 10000);
	    sz = this->height()-2*frame->frameWidth();
	  }else{
	    this->setMaximumSize(10000, trayIcons.length()*this->width());
	    sz = this->width()-2*frame->frameWidth();
	  }
	  if(sz>64){ sz = 64; } //many tray icons can't go larger than this anyway
	  for(int i=0; i<trayIcons.length(); i++){
	    trayIcons[i]->setSizeSquare(sz);
	    trayIcons[i]->repaint();
	  }
	}
	
};

#endif
