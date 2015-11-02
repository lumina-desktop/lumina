//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This class is the sample plugin for a ScreenSaver animation
//===========================================
#ifndef _LUMINA_DESKTOP_SCREEN_SAVER_SAMPLE_ANIMATION_H
#define _LUMINA_DESKTOP_SCREEN_SAVER_SAMPLE_ANIMATION_H

#include "GlobalDefines.h"
#include "BaseAnimGroup.h"

class SampleAnimation : public BaseAnimGroup{
	Q_OBJECT
public:
	SampleAnimation(QWidget *parent, QSettings *set) : BaseAnimGroup(parent, set){}
	
	~SampleAnimation(){}
		
private:
	QWidget *ball;
	virtual void LoadAnimations(){
	  ball = new QWidget(canvas);
	  //This creates a red "ball" on the widget which is going to expand/contract in the center of the screen
	  ball->setStyleSheet("background:  qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.341, fy:0.796, stop:0.00531915 rgba(107, 10, 10, 255), stop:0.521277 rgba(170, 10, 10, 255), stop:0.957447 rgba(200, 0, 0, 255), stop:0.994681 rgba(0, 0, 0, 225), stop:1 rgba(255, 255, 255, 0));");
	  //ball->setSize(QSize(64,64));
	  ball->move(canvas->geometry().center()-QPoint(32,32));
	  //Now setup the movements
	  QPropertyAnimation *move = new QPropertyAnimation(ball,"size");
	   move->setKeyValueAt(0, QSize(64, 64) );
	   int size = canvas->width();
	    if(size > canvas->height()){ size = canvas->height(); }
	   move->setKeyValueAt(0.5, QSize(size,size)); //touch the edge of the screen
	   move->setKeyValueAt(1, QSize(64, 64) ); //back to original size
	   move->setDuration(10000); //10 seconds
	   this->addAnimation(move);
	   this->setLoopCount(10); //repeat 10 times
	}

};
#endif
