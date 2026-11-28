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

#include "global-includes.h"
#include "BaseAnimGroup.h"

class SampleAnimation : public BaseAnimGroup{
	Q_OBJECT
private:
	QWidget *ball;

public:
	SampleAnimation(QWidget *parent) : BaseAnimGroup(parent){}
	~SampleAnimation(){ this->stop(); delete ball; }

	void LoadAnimations(){
	  //qDebug() << "Loading Sample Animation";
	  ball = new QWidget(canvas);
	  //This creates a red "ball" on the widget which is going to expand/contract in the center of the screen
	  ball->setStyleSheet("background:  qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.341, fy:0.796, stop:0.00531915 rgba(107, 10, 10, 255), stop:0.521277 rgba(170, 10, 10, 255), stop:0.957447 rgba(200, 0, 0, 255), stop:0.994681 rgba(0, 0, 0, 225), stop:1 rgba(255, 255, 255, 0));");
	  //Now setup the movements
	  QPropertyAnimation *move = new QPropertyAnimation(ball,"geometry");
	    QPoint ctr(canvas->width()/2, canvas->height()/2);
	    QRect initgeom(ctr-QPoint(12,12), QSize(24,24) );
	   move->setKeyValueAt(0, initgeom ); //starting point
	   move->setKeyValueAt(1, initgeom ); //ending point (same as start for continuity)
	   int size = canvas->width();
	    if(size > canvas->height()){ size = canvas->height(); }
	   move->setKeyValueAt(0.5, QRect(ctr-QPoint(size/2, size/2), QSize(size,size))); //touch the edge of the screen
	   move->setDuration(10000); //10 seconds
	   this->addAnimation(move);
	   this->setLoopCount(10); //repeat 10 times
	   ball->show();
	}

};
#endif
