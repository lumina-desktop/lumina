//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_SCREEN_SAVER_GRAV_ANIMATION_H
#define _LUMINA_DESKTOP_SCREEN_SAVER_GRAV_ANIMATION_H

//PI is equal to 2*pi
#define PI 6.2832
#include "global-includes.h"
#include "BaseAnimGroup.h"
#include <QtMath>
#include <QMatrix>

class Grav: public QParallelAnimationGroup{
	Q_OBJECT
private:
	QWidget *planet;
	QPropertyAnimation *orbit;
	QSize range;
	//QList<QPoint> path;
	double radius;

	void setupLoop(QPoint start, QPoint *ref){
	  //orbit->setStartValue(start);

	  //Used to find overall speed. Distance from the planet to the sun
	  radius = qSqrt( (qPow(start.x()-ref->x(),2) + qPow(start.y()-ref->y(),2) ));

	  //Number of frames in animation. Increase for smother motion
	  double step = 300.0;

	  //Random values that give the eliptical pattern to the orbit. Between 0.4 and 2.3
   	  double xrand = 0.4; //(qrand()%10+4)/10.0;
	  double yrand = 0.4; //(qrand()%10+4)/10.0;

	  double theta = 1.5707963;
	  //double theta = aTan((start.x() - ref->x())/(start.y() - ref->y()));
	  QMatrix rotation = QMatrix(qCos(theta), qSin(theta), -qSin(theta), qCos(theta), -ref->x(), -ref->y());
	  qDebug() << rotation;
	  //qDebug() << "Starting Point" << start;
	  //qDebug() << "Angle" << theta;
	  //qDebug() << "Distance" << radius;
	  //qDebug() << "Center" << *ref;

	  QPoint firstP = (QPoint(ref->x() + xrand*start.x()*(qCos(0/step) -qSin(0/step)), ref->y() + yrand*start.y()*(qCos(0/step) -qSin(0/step))));
	  QPoint rotFP = rotation.map(firstP);
	  qDebug() << "First Point" << firstP;
	  qDebug() << "Rotation by Matrix" << rotFP;
	  QPoint lastP = (QPoint(ref->x() + xrand*start.x()*(qCos(PI/step) -qSin(PI/step)), ref->y() + yrand*start.y()*(qCos(PI/step) -qSin(PI/step))));
	  orbit->setKeyValueAt(0, firstP);
	  orbit->setKeyValueAt(1, lastP);
	  //path.push_back(firstP);

	  //Loops through all steps and creates all the points of the orbit
	  for(int i=1; i<step; i++) {
		//Calculates the new point, including gravitational pull and eccentricity. Goes from 0 to 2PI in steps.
		double newX = ref->x() + xrand*start.x()*(qCos((PI*i)/step) - qSin((PI*i)/step));
		double newY = ref->y() + yrand*start.y()*(qSin((PI*i)/step) + qCos((PI*i)/step));

		//Creates a new point and creates a key as part of the animation
		QPoint newLoc = (QPoint(newX, newY));
		orbit->setKeyValueAt(i/step, newLoc);
		//path.push_back(newLoc);
	  }
	  //Sets the time for a full orbit. Increasing makes the orbit slower.
	  //path.push_back(lastP);
	}
private slots:
	/*void LoopChanged(int loop){
	    //Adjust the orbit animation a bit
	    if(loop >= 0) {
	      orbit->setStartValue(orbit->endValue()); //start at the previous end point
              orbit->setEndValue(path.at(loop%1000));
	      orbit->setDuration(10);
	    }
	}*/
	void stopped(){ planet->hide();}

public:
	Grav(QWidget *parent) : QParallelAnimationGroup(parent){
	  planet = new QWidget(parent);
	  range = parent->size();
	  QPoint center = QRect(QPoint(0,0), parent->size()).center();;

	  //Creates a random planet size. Between 12 and 45 pixels
	  int planet_radius = qRound(1.75* ((qrand()%20)+7) );

	  //Creates a random color in RGB, then creates a circular gradient
	  QString color = "rgba(" + QString::number(qrand() % 256) + ", " + QString::number(qrand() % 256) + ", " + QString::number(qrand() % 256);
	  QString style = "background-color: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 " + color+
					", 255)" + " , stop:0.83871 " + color + ", 140)" + " , stop:0.99 rgba(0, 0, 0, 255), stop:1 transparent);";
 	  planet->setStyleSheet(style);

	  //setup the orbit animation
	  orbit = new QPropertyAnimation(planet);
	  orbit->setPropertyName("pos");
	  orbit->setTargetObject(planet);

	  //Creates the random position of the planet, making sure it isn't too close to the sun
	  QRect invalid = QRect(center+QPoint(-50,-50), center+QPoint(50,50));
	  QPoint tmp = center;
	  while(invalid.contains(tmp)){
 	    int randwidth = qrand()%(range.width() - 2*planet_radius) + planet_radius;
 	    int randheight = qrand()%(range.height()- 2*planet_radius) + planet_radius;
	    tmp = QPoint(randwidth, randheight);
	  }

	  //Creates all frames for the animation
	  setupLoop(tmp, &center);
	  this->addAnimation(orbit);
	  planet->show();

	  //Ensures the screensaver will not stop until the user wishes to login or it times out
	  this->setLoopCount(3); //number of orbits
	  orbit->setDuration( qrand() %1000 + 19000); //20 second orbits
	  //orbit->setEndValue(path.at(0));
	  //LoopChanged(0);  //load initial values

	  //Sets the initial size and location of the planet
	  planet->setGeometry(QRect(orbit->startValue().toPoint(), QSize(planet_radius, planet_radius)));
	  //connect(this, SIGNAL(currentLoopChanged(int)), this, SLOT(LoopChanged(int)) );
	  connect(this, SIGNAL(finished()), this, SLOT(stopped()) );
	}
	~Grav(){}

};

class GravAnimation : public BaseAnimGroup{
	Q_OBJECT
private:
	QList<Grav*> planets;
	QWidget *sun;
	QPropertyAnimation *wobble;

private slots:
	void checkFinished(){
	  int running = 0;
	  for(int i=0; i<this->animationCount(); i++){
	    if(this->animationAt(i)->state()==QAbstractAnimation::Running){ running++; }
	  }
	  if(running<=1){ wobble->stop();  emit wobble->finished();}
	}

public:
	GravAnimation(QWidget *parent) : BaseAnimGroup(parent){}
	~GravAnimation(){
	  sun->deleteLater();
	  while(planets.length()>0){ planets.takeAt(0)->deleteLater(); }
	}

	void LoadAnimations(){
	  //Creates the sun, which is a thin shell with a gradient from green to yellow
	  sun = new QWidget(canvas);
	  QPoint center = QRect(QPoint(0,0), canvas->size()).center();
	  QString sunstyle = QStringLiteral("background-color:qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, ") +
						QStringLiteral("stop:0 rgba(0, 0, 0, 0), stop:0.38 rgba(0, 0, 0, 0), stop:0.4 rgba(82, 121, 76, 33), stop:0.5 rgba(159, 235, 148, 64), ") +
						QStringLiteral("stop:0.6 rgba(255, 238, 150, 129), stop:0.7 rgba(0, 0, 0, 0));");
	  sun->setStyleSheet(sunstyle);

	  //Creates the sun's pulsing animation
	  wobble = new QPropertyAnimation(sun);
	  wobble->setPropertyName("geometry");
	  wobble->setTargetObject(sun);
	  QRect initgeom = QRect(center-QPoint(30,30), QSize(60, 60));
	  wobble->setStartValue(initgeom);
	  wobble->setKeyValueAt(0, initgeom ); //starting point
	  wobble->setKeyValueAt(1, initgeom ); //starting point
	  wobble->setKeyValueAt(0.5, QRect(center-QPoint(45,45), QSize(90, 90))); //starting point
	  wobble->setDuration(2000);
	  wobble->setLoopCount(-1);
	  this->addAnimation(wobble);
	  sun->show();
	  sun->setGeometry(initgeom);

	  //Gives the screensaver a black background
	  //canvas->setStyleSheet("background: black;");

	  //Pulls number of planets from settings, with 10 as default
	  int number = readSetting("planets/number",qrand()%5+3).toInt();

	  //Loops through all planets and sets up the animations, then adds them to the base group and vector, which
	  //qDebug() << "Starting planets";
	  for(int i=0; i<number; i++){
	    Grav *tmp = new Grav(canvas);
	    this->addAnimation(tmp);
		connect(tmp, SIGNAL(finished()), this, SLOT(checkFinished()));
	    planets << tmp;
	  }
	}

};
#endif
