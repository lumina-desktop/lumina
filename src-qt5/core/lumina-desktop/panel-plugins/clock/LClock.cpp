//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LClock.h"
#include "LSession.h"
#include <LuminaThemes.h>
#include <LuminaXDG.h>

LClock::LClock(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setToolButtonStyle(Qt::ToolButtonTextOnly);
    button->setStyleSheet("font-weight: bold;");
    button->setPopupMode(QToolButton::DelayedPopup); //make sure it runs the update routine first
    button->setMenu(new QMenu());
	//button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	//this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connect(button, SIGNAL(clicked()), this, SLOT(openMenu()));
    connect(button->menu(), SIGNAL(aboutToHide()), this, SIGNAL(MenuClosed()));
  calendar = new QCalendarWidget(this);
  calAct = new QWidgetAction(this);
	calAct->setDefaultWidget(calendar);
  TZMenu = new QMenu(this);
    connect(TZMenu, SIGNAL(triggered(QAction*)), this, SLOT(ChangeTZ(QAction*)) );
	
  //Now assemble the menu
  button->menu()->addAction(calAct);
  button->menu()->addMenu(TZMenu);
	
  this->layout()->setContentsMargins(0,0,0,0); //reserve some space on left/right
  this->layout()->addWidget(button);
	
  //Setup the timer
  timer = new QTimer();
  //Load all the initial settings
  updateFormats();
  LocaleChange();
  ThemeChange();
  OrientationChange();
  //Now connect/start the timer
  connect(timer,SIGNAL(timeout()), this, SLOT(updateTime()) );
  connect(QApplication::instance(), SIGNAL(SessionConfigChanged()), this, SLOT(updateFormats()) );
  timer->start();
}

LClock::~LClock(){
  timer->stop();
  delete timer;
}


void LClock::updateTime(bool adjustformat){
  QDateTime CT = QDateTime::currentDateTime();
  //Now update the display
  QString label;
  QString timelabel;
  QString datelabel;
  if(deftime){ timelabel = CT.time().toString(Qt::DefaultLocaleShortDate) ; }
  else{ timelabel=CT.toString(timefmt); }
  if(defdate){ datelabel = CT.date().toString(Qt::DefaultLocaleShortDate); }
  else{ datelabel = CT.toString(datefmt); }
  if(datetimeorder == "dateonly"){
	  label = datelabel;
	  button->setToolTip(timelabel);
  }else if(datetimeorder == "timedate"){
	  label = timelabel + "\n" + datelabel;
	  button->setToolTip("");
  }else if(datetimeorder == "datetime"){
	  label = datelabel + "\n" + timelabel;
	  button->setToolTip("");
  }else{ 
	 label = timelabel;
	button->setToolTip(datelabel);
  }
  if( this->layout()->direction() == QBoxLayout::TopToBottom ){
    //different routine for vertical text (need newlines instead of spaces)
    label.replace(" ","\n");
  }
  if(adjustformat){
   //Check the font/spacing for the display and adjust as necessary
    /*double efflines = label.count("\n")+1; //effective lines (with wordwrap)
    if( (button->fontMetrics().height()*efflines) > button->height() ){
      //Force a pixel metric font size to fit everything
      int szH = qRound( (button->height() - button->fontMetrics().lineSpacing() )/efflines );
      //Need to supply a *width* pixel, not a height metric
      int szW = qRound( (szH*button->fontMetrics().maxWidth())/( (double) button->fontMetrics().height()) );
      qDebug() << "Change Clock font:" << button->height() << szH << szW << efflines << button->fontMetrics().height() << button->fontMetrics().lineSpacing();
      button->setStyleSheet("font-weight: bold; font-size: "+QString::number(szW)+"px;");
    }else{
      button->setStyleSheet("font-weight: bold;");
    }*/
  }
  button->setText(label);

}

void LClock::updateFormats(){
  qDebug() << "Updating clock format";
  timefmt = LSession::handle()->sessionSettings()->value("TimeFormat","").toString();
  datefmt = LSession::handle()->sessionSettings()->value("DateFormat","").toString();
  deftime = timefmt.simplified().isEmpty();
  defdate = datefmt.simplified().isEmpty();
  //Adjust the timer interval based on the smallest unit displayed
  if(deftime){ timer->setInterval(500); } //1/2 second
  else if(timefmt.contains("z")){ timer->setInterval(1); } //every millisecond (smallest unit)
  else if(timefmt.contains("s")){ timer->setInterval(500); } //1/2 second
  else if(timefmt.contains("m")){ timer->setInterval(2000); } //2 seconds
  else{ timer->setInterval(1000); } //unknown format - use 1 second interval
  datetimeorder = LSession::handle()->sessionSettings()->value("DateTimeOrder", "timeonly").toString().toLower();
  //this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  updateTime(true);
  //Now fix the size of the widget with the new size hint
  this->setFixedWidth( this->sizeHint().width() +6);
}

void LClock::updateMenu(){
  QDateTime cdt = QDateTime::currentDateTime();
  TZMenu->setTitle(QString(tr("Time Zone (%1)")).arg(cdt.timeZoneAbbreviation()) );
  calendar->showToday(); //make sure the current month is visible
  calendar->setSelectedDate(QDate::currentDate()); //select the actual date for today
}

void LClock::openMenu(){
  updateMenu();
  button->showMenu();
}

void LClock::closeMenu(){
  button->menu()->hide();
}
	
void LClock::ChangeTZ(QAction *act){
  LTHEME::setCustomEnvSetting("TZ",act->whatsThis());
  QTimer::singleShot(500, this, SLOT(updateTime()) );
}

void LClock::LocaleChange(){
  //Refresh all the time zone information
  TZMenu->clear();
    TZMenu->addAction(tr("Use System Time"));
    TZMenu->addSeparator();
  QList<QByteArray> TZList = QTimeZone::availableTimeZoneIds();
  //Orgnize time zones for smaller menus (Continent/Country/City)
  // Note: id = Continent/City
  QStringList info;
  for(int i=0; i<TZList.length(); i++){
    QTimeZone tz(TZList[i]);
    if(!QString(tz.id()).contains("/")){ continue; }
    info << "::::"+QString(tz.id()).section("/",0,0)+"::::"+QLocale::countryToString(tz.country())+"::::"+QString(tz.id()).section("/",1,100).replace("_"," ")+"::::"+QString(tz.id());
  }
  //Now sort alphabetically
  info.sort();
  //Now create the menu tree
  QString continent, country; //current continent/country
  QMenu *tmpC=0; //continent menu
  QMenu *tmpCM=0; //country menu
  for(int i=0; i<info.length(); i++){
    //Check if different continent
    if(info[i].section("::::",1,1)!=continent){
      if(tmpC!=0){
        if(tmpCM!=0 && !tmpCM->isEmpty()){
	  tmpC->addMenu(tmpCM);
	}
	if(!tmpC->isEmpty()){ TZMenu->addMenu(tmpC); }
      }
      tmpC = new QMenu(this);
	tmpC->setTitle(info[i].section("::::",1,1));
      tmpCM = new QMenu(this);
	  tmpCM->setTitle(info[i].section("::::",2,2));
    //Check if different country
    }else if(info[i].section("::::",2,2)!=country){
        if(tmpC!=0 && tmpCM!=0 && !tmpCM->isEmpty()){
	  tmpC->addMenu(tmpCM);
	}
	tmpCM = new QMenu(this);
	  tmpCM->setTitle(info[i].section("::::",2,2));
    }
    //Now create the entry within the country menu
    if(tmpCM!=0){
      QAction *act = new QAction(info[i].section("::::",3,3), this);
	act->setWhatsThis(info[i].section("::::",4,4) );
      tmpCM->addAction(act);
    }
    //Save the values for the next run
    continent = info[i].section("::::",1,1);
    country = info[i].section("::::",2,2);

    if(i== info.length()-1){
      //last go through - save all menus
      if(tmpCM!=0 && tmpC!=0 && !tmpCM->isEmpty()){ tmpC->addMenu(tmpCM); }
      if(tmpC!=0 && !tmpC->isEmpty()){ TZMenu->addMenu(tmpC); }
    }
  }
  
}

void LClock::ThemeChange(){
  TZMenu->setIcon(LXDG::findIcon("clock",""));
}

void LClock::OrientationChange(){
  if(this->layout()->direction()==QBoxLayout::LeftToRight){
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  }else{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  }
  updateTime(true); //re-adjust the font/spacings
  this->layout()->update();
}
