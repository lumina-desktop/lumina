//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_SCREEN_SAVER_GRAV_ANIMATION_H
#define _LUMINA_DESKTOP_SCREEN_SAVER_GRAV_ANIMATION_H

#define PI 3.1416

#include "global-includes.h"
#include "BaseAnimGroup.h"
#include <QSequentialAnimationGroup>
#include <cmath>

class Grav: public QSequentialAnimationGroup{
	Q_OBJECT
private:
	QWidget *planet;
	QPropertyAnimation *movement;
	QSize range;
private slots:
	void LoopChanged(){
	    static double time = 0;
	    //Adjust the movement animation a bit
	    movement->setStartValue(movement->endValue()); //start at the previous end point
	    QPoint pt = movement->startValue().toPoint();
	    QPoint diff((cos(pt.x()) - cos(pt.x()+time)), (sin(pt.y()) - sin(pt.y() +time)) );
	    pt.setX( pt.x() + diff.x() );
	    pt.setY( pt.y() + diff.y() );
	    movement->setEndValue(pt);
	    movement->setDuration(2);
	    time+=0.01;
	  planet->show();
	}
	void stopped(){planet->hide(); }

public:
	Grav(QWidget *parent) : QSequentialAnimationGroup(parent){
	  planet = new QWidget(parent);
	  range = parent->size();
	  planet->setStyleSheet("background-color: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 rgba(215, 215, 143, 255), stop:0.83871 rgba(221, 235, 64, 140), stop:1 rgba(0, 0, 0, 255));");
	  //setup the movement animation
	  movement = new QPropertyAnimation(planet);
	  movement->setTargetObject(planet);
	  movement->setPropertyName("pos");
	  movement->setEndValue( QPoint( qrand() % range.width(), qrand()%range.height()) ); //on anim start, this will become the starting point
	  this->addAnimation(movement);
	  LoopChanged();  //load initial values
	  planet->setGeometry( QRect(movement->startValue().toPoint(), QSize(15, 15)));
	  connect(this, SIGNAL(currentLoopChanged(int)), this, SLOT(LoopChanged()) );
	  connect(this, SIGNAL(finished()), this, SLOT(stopped()) );
	}
	~Grav(){}

};

class GravAnimation : public BaseAnimGroup{
	Q_OBJECT
private:
	QList<Grav*> planets;

public:
	GravAnimation(QWidget *parent, QSettings *set) : BaseAnimGroup(parent, set){}
	~GravAnimation(){
	  this->stop();
	}

	void LoadAnimations(){
	  while(planets.length()>0){ planets.takeAt(0)->deleteLater(); }
	  canvas->setStyleSheet("background: black;");
	  int number = settings->value("planets/number",10).toInt();
	  for(int i=0; i<number; i++){
            if(planets.length()>number){ continue; }
	    Grav *tmp = new Grav(canvas);
	    this->addAnimation(tmp);
	    planets << tmp;
	  }
	  while(planets.length()>number){planets.takeAt(number)->deleteLater(); }
	}

};
#endif
