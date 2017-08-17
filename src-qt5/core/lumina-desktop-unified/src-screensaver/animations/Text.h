//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_SCREEN_SAVER_TEXT_ANIMATION_H
#define _LUMINA_DESKTOP_SCREEN_SAVER_TEXT_ANIMATION_H

#include "global-includes.h"
#include "BaseAnimGroup.h"
#include <QParallelAnimationGroup>
#include <QtMath>

class Text: public QParallelAnimationGroup{
	Q_OBJECT
private:
	QLabel *text;
	QPropertyAnimation *movement;
	QSize range;
	QPoint v;
	bool bounce;

private slots:
	void LoopChanged(){
	  movement->setStartValue(movement->endValue());
	  QPoint currLoc = movement->startValue().toPoint();
	  bounce = !(currLoc.y() < 100 or currLoc.y() > range.height()-100 or currLoc.x() > range.width()-100 or currLoc.x() < 100);
	  if((currLoc.y() < 10 or currLoc.y() > range.height()-40) and !bounce) {
	    v.setY((v.y() * -1) + (qrand() % 20 - 10));
	  }else if((currLoc.x() > range.width()-10 or currLoc.x() < 10) and !bounce) {
	    v.setX((v.x() * -1) + (qrand() % 20 - 10));
	  }
	  currLoc.setX(currLoc.x() + v.x());
	  currLoc.setY(currLoc.y() + v.y());
	  movement->setEndValue(currLoc);
	}
	void stopped(){ qDebug() << "Text Stopped"; text->hide();}

public:
	Text(QWidget *parent) : QParallelAnimationGroup(parent){
	  text = new QLabel(parent);
	  range = parent->size();
	  QPoint center = parent->geometry().center();

	  QString color = "rgba(" + QString::number(qrand() % 206 + 50) + ", " + QString::number(qrand() % 206 + 50) + ", " + QString::number(qrand() % 206 + 50);
	  text->setStyleSheet("QLabel {background-color: rgba(255, 255, 255, 10); color: " + color + "); }");
	  text->setFont(QFont("Courier", 24, QFont::Bold));
	  text->setText("test");
          QFontMetrics metrics(text->font());
	  text->setMinimumSize(QSize( metrics.width(text->text())+10, metrics.height()*text->text().count("\n") +10));

	  movement = new QPropertyAnimation(text);
	  movement->setPropertyName("pos");
	  movement->setTargetObject(text);

	  this->addAnimation(movement);
	  text->show();
	  v.setX((qrand() % 100 + 50) * qPow(-1, qrand() % 2));
	  v.setY((qrand() % 100 + 50) * qPow(-1, qrand() % 2));
	  movement->setStartValue(center);
	  //Ensures the screensaver will not stop until the user wishes to login or it times out
	  this->setLoopCount(2000); //number of movements
	  movement->setDuration(200);
	  movement->setEndValue(QPoint(qrand() % (int)range.height(), qrand() % range.width()));
	  LoopChanged();  //load initial values

	  connect(this, SIGNAL(currentLoopChanged(int)), this, SLOT(LoopChanged()) );
	  connect(this, SIGNAL(finished()), this, SLOT(stopped()) );
	}
	~Text(){}

};

class TextAnimation : public BaseAnimGroup{
	Q_OBJECT
public:
	TextAnimation(QWidget *parent, QSettings *set) : BaseAnimGroup(parent, set){}
	~TextAnimation(){
	  this->stop();
	}

	void LoadAnimations(){
	  canvas->setStyleSheet("background: black;");
	  Text *tmp = new Text(canvas);
	  this->addAnimation(tmp);
	}

};
#endif
