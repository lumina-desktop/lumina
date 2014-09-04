//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LClock.h"

LClock::LClock(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  //Setup the widget
  label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
  this->layout()->setContentsMargins(3,0,3,0); //reserve some space on left/right
  this->layout()->addWidget(label);
	
  //Setup the timer
  timer = new QTimer();
  timer->setInterval(1000); //update once a second
  connect(timer,SIGNAL(timeout()), this, SLOT(updateTime()) );
  updateTime();
  timer->start();
}

LClock::~LClock(){
  timer->stop();
  delete timer;
}

void LClock::updateTime(){
  QDateTime CT = QDateTime::currentDateTime();
  //Now update the display
  QLocale sys = QLocale::system();
  label->setText( "<b>"+CT.toString(sys.timeFormat(QLocale::ShortFormat))+"</b>" );
  label->setToolTip(CT.toString(sys.dateFormat()));
}
