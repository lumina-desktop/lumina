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
  labelWidget = new QLabel(this);
    labelWidget->setAlignment(Qt::AlignCenter);
    labelWidget->setStyleSheet("font-weight: bold;");
    labelWidget->setWordWrap(true);
  this->layout()->setContentsMargins(3,0,3,0); //reserve some space on left/right
  this->layout()->addWidget(labelWidget);
	
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
  QString label;
  if(deftime){ label = CT.time().toString(Qt::SystemLocaleShortDate) ; }
  else{ label=CT.toString(timefmt); }
  if(defdate){ labelWidget->setToolTip(CT.date().toString(Qt::SystemLocaleLongDate)); }
  else{ labelWidget->setToolTip(CT.toString(datefmt)); }
  if( this->layout()->direction() == QBoxLayout::TopToBottom ){
    //different routine for vertical text (need newlines instead of spaces)
    label.replace(" ","\n");
  }
  labelWidget->setText(label);
}

void LClock::updateFormats(){
  timefmt = LSession::handle()->sessionSettings()->value("TimeFormat","").toString();
  datefmt = LSession::handle()->sessionSettings()->value("DateFormat","").toString();
  deftime = timefmt.simplified().isEmpty();
  defdate = datefmt.simplified().isEmpty();
}

