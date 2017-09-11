//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This class is the sample plugin for a ScreenSaver animation
//===========================================
#ifndef _LUMINA_DESKTOP_SCREEN_SAVER_FIREFLIES_ANIMATION_H
#define _LUMINA_DESKTOP_SCREEN_SAVER_FIREFLIES_ANIMATION_H

#include "global-includes.h"
#include "BaseAnimGroup.h"
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>

class Firefly : public QSequentialAnimationGroup{
	Q_OBJECT
private:
	QWidget *fly;
	QPropertyAnimation *movement, *flash;
	int maxX, maxY; //maximum jitter in X/Y directions
	QSize range;
private slots:
	void LoopChanged(){
	    //Adjust the movement animation a bit
	    movement->setStartValue(movement->endValue()); //start at the previous end point
	    QPoint pt = movement->startValue().toPoint();
	    QPoint diff( (qrand()% maxX) - (maxX/2), (qrand()% maxY) - (maxY/2) );
	    //Need to ensure it stays in the current box
	    if( (pt.x()+diff.x()) < 0 || (pt.x()+diff.x())>range.width()){ pt.setX(pt.x() - diff.x()); } //reverse the direction - otherwise will go out of bounds
	    else{ pt.setX( pt.x() + diff.x() ); }
	    if( (pt.y()+diff.y()) < 0 || (pt.y()+diff.y())>range.height()){ pt.setY(pt.y() - diff.y()); } //reverse the direction - otherwise will go out of bounds
	    else{ pt.setY( pt.y() + diff.y() ); }
	    movement->setEndValue(pt);
	    movement->setDuration( qrand() %500 + 1000); //between 1000->1500 ms animations for movements
	    //Adjust the flash duration/size a bit
	    flash->setDuration(qrand() %200 + 500); //500-700 ms
	    int sz = qrand()%4 + 4; //6-10 pixel square
	    //flash->setKeyValueAt(0.5, (qrand()%50) /100.0);
	    //fly->resize(sz,sz);
	    flash->setKeyValueAt(0.5, QSize(sz,sz)); //half-way point for the flash

	  fly->show();
	}
	void stopped(){ fly->hide(); }

public:
	Firefly(QWidget *parent) : QSequentialAnimationGroup(parent){
	  fly = new QWidget(parent);
	  range = parent->size();
	  maxX = range.width()/4;  maxY = range.height()/4;
	  QString B = QString::number(qrand()%70);
          QString RY = QString::number(qrand()%200+50);
          QString style = "background-color: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 rgba(245, 245, 143, 200), stop:0.83871 rgba(%1, %1, %2, 140), stop:0.99 rgba(0, 0, 0, 255), stop:1 transparent);";
	  fly->setStyleSheet(style.arg(RY, B) );
	  //setup the movement animation
	  movement = new QPropertyAnimation(fly);
	  movement->setTargetObject(fly);
	  movement->setPropertyName("pos");
	  movement->setEndValue( QPoint( qrand() % range.width(), qrand()%range.height()) ); //on anim start, this will become the starting point
	  //setup the flashing animation
	  /*QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(parent);
	  fly->setGraphicsEffect(eff);
	  flash = new QPropertyAnimation(eff, "opacity");*/
	  flash = new QPropertyAnimation(this);
	  flash->setTargetObject(fly);
	  flash->setPropertyName("size");
	  flash->setStartValue(QSize(0,0));
	  flash->setEndValue(flash->startValue());
	  //fly->setProperty("opacity",0);
	  //flash->setPropertyName("opacity");
	  //flash->setStartValue(0);
	  //flash->setEndValue(0);
	  //now setup the order of the animations
	  this->setLoopCount(100); //do this 100 times
	  //Roughly half the number of fireflies with start with movement/flash
          if(qrand()%2 == 1){
	    this->addAnimation(movement);
	    this->addAnimation(flash);
	  }else{
	    this->addAnimation(flash);
	    this->addAnimation(movement);
	  }
	  //Start up this firefly
	  LoopChanged();  //load initial values

	  fly->setGeometry( QRect(movement->startValue().toPoint(), flash->startValue().toSize()) );
	  connect(this, SIGNAL(currentLoopChanged(int)), this, SLOT(LoopChanged()) );
	  connect(this, SIGNAL(finished()), this, SLOT(stopped()) );
	}
	~Firefly(){}

};

class FirefliesAnimation : public BaseAnimGroup{
	Q_OBJECT
private:
	QList<Firefly*> fireflies;

public:
	FirefliesAnimation(QWidget *parent) : BaseAnimGroup(parent){}
	~FirefliesAnimation(){
	  this->stop();
	  //while(fireflies.length()>0){ fireflies.takeAt(0)->deleteLater(); }
	}

	void LoadAnimations(){
	  while(fireflies.length()>0){ fireflies.takeAt(0)->deleteLater(); }
	  canvas->setStyleSheet("background: black;");
	  int number = readSetting("number",qrand()%30 + 50).toInt();
	  for(int i=0; i<number; i++){
            if(fireflies.length()>number){ continue; }
	    Firefly *tmp = new Firefly(canvas);
	    this->addAnimation(tmp);
	    fireflies << tmp;
	  }

	}

};
#endif
