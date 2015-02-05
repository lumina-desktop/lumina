//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LClock.h"

#include "LSession.h"

LClock::LClock(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  //Setup the widget
  label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
  this->layout()->setContentsMargins(3,0,3,0); //reserve some space on left/right
  this->layout()->addWidget(label);
	
  //Setup the timer
  timer = new QTimer();
    timer->setInterval(1000); //update once a second
  updateFormats();
  updateTime();
  connect(timer,SIGNAL(timeout()), this, SLOT(updateTime()) );
  connect(QApplication::instance(), SIGNAL(SessionConfigChanged()), this, SLOT(updateFormats()) );
  timer->start();
}

LClock::~LClock(){
  timer->stop();
  delete timer;
}


void LClock::updateTime(){
  QDateTime CT = QDateTime::currentDateTime();
  //Now update the display
  if(deftime){ label->setText( "<b>"+CT.time().toString(Qt::SystemLocaleShortDate)+"</b>" ); }
  else{ label->setText( "<b>"+CT.toString(timefmt)+"</b>" ); }
  if(defdate){ label->setToolTip(CT.date().toString(Qt::SystemLocaleLongDate)); }
  else{ label->setToolTip(CT.toString(datefmt)); }
}

void LClock::updateFormats(){
  timefmt = LSession::handle()->sessionSettings()->value("TimeFormat","").toString();
  datefmt = LSession::handle()->sessionSettings()->value("DateFormat","").toString();
  deftime = timefmt.simplified().isEmpty();
  defdate = datefmt.simplified().isEmpty();
}