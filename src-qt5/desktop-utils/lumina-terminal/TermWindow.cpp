//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "TermWindow.h"
//#include "ui_TermWindow.h"

#include <QDesktopWidget>
#include <QDebug>
#include <QTimer>
#include <QApplication>
#include <QVBoxLayout>
#include "TerminalWidget.h"

// ===============
//        PUBLIC
// ===============
TermWindow::TermWindow(QSettings *set) : QWidget(0, Qt::Window | Qt::BypassWindowManagerHint){//, ui(new Ui::TermWindow){
  this->setWindowOpacity(0.85);
  CLOSING = false; //internal flag
  settings = set;
  //Create the Window
  this->setLayout(new QVBoxLayout());
  this->setCursor(Qt::SplitVCursor);
  tabWidget = new QTabWidget(this);
    tabWidget->clear(); //just in case
    tabWidget->setCursor(Qt::ArrowCursor);
    tabWidget->setTabBarAutoHide(true);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    tabWidget->setUsesScrollButtons(true);
    this->layout()->addWidget(tabWidget);
  //Setup the animation
  ANIM = new QPropertyAnimation(this, "geometry", this);
    ANIM->setDuration(300); //1/3 second animation
  connect(ANIM, SIGNAL(finished()), this, SLOT(AnimFinished()) );
  activeTimer = new QTimer(this);
    activeTimer->setInterval(50);
    activeTimer->setSingleShot(true);
    connect(activeTimer, SIGNAL(timeout()), this, SLOT(activeStatusChanged()) );
    connect(QApplication::instance(), SIGNAL(applicationStateChanged(Qt::ApplicationState)), activeTimer, SLOT(start()) );
  //Create the keyboard shortcuts
  //hideS = new QShortcut(QKeySequence(Qt::Key_Escape),this);
  closeS = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q),this);
  newTabS = new QShortcut(QKeySequence::AddTab,this);
  closeTabS = new QShortcut(QKeySequence::Close,this);
  prevTabS = new QShortcut(QKeySequence::PreviousChild,this);
  nextTabS = new QShortcut(QKeySequence::NextChild,this);
  //Print out all the keyboard shortcuts onto the screen
  qDebug() << "New Tab Shortcut:" << QKeySequence::keyBindings(QKeySequence::AddTab);
  qDebug() << "Close Tab Shortcut:" << QKeySequence::keyBindings(QKeySequence::Close);
  qDebug() << "Next Tab Shortcut:" << QKeySequence::keyBindings(QKeySequence::NextChild);
  qDebug() << "Previous Tab Shortcut:" << QKeySequence::keyBindings(QKeySequence::PreviousChild);
  //Connect the signals/slots
  connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(Close_Tab(int)) );
  connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(focusOnWidget()) );
  connect(closeTabS, SIGNAL(activated()), this, SLOT(Close_Tab()) );
  connect(newTabS, SIGNAL(activated()), this, SLOT(New_Tab()) );
  //connect(hideS, SIGNAL(activated()), this, SLOT(HideWindow()) );
  connect(closeS, SIGNAL(activated()), this, SLOT(CloseWindow()) );
  connect(prevTabS, SIGNAL(activated()), this, SLOT(Prev_Tab()) );
  connect(nextTabS, SIGNAL(activated()), this, SLOT(Next_Tab()) );
  //Now set the defaults
  screennum = 0; //default value
  setTopOfScreen(true); //default value
  if(settings->contains("lastSize")){ 
    //qDebug() << "Re-use last size:" << settings->value("lastSize").toSize();
    this->resize( settings->value("lastSize").toSize() ); 
    CalculateGeom();
    //qDebug() << "After size:" << this->size();
  }
  
  //this->resize(this->width(),300);
  //this->setMinimumSize(20, 300);
  
}


TermWindow::~TermWindow(){
	
}

void TermWindow::cleanup(){
  //called right before the window is closed
  //Make sure to close any open tabs/processes
  CLOSING = true;
  for(int i=0; i<tabWidget->count(); i++){
    static_cast<TerminalWidget*>(tabWidget->widget(i))->aboutToClose();
  }
}

void TermWindow::OpenDirs(QStringList dirs){
  for(int i=0; i<dirs.length(); i++){
    //Open a new tab for each directory
    TerminalWidget *page = new TerminalWidget(tabWidget, dirs[i]);
    QString ID = GenerateTabID();
      page->setWhatsThis(ID);
    tabWidget->addTab(page, ID);
    tabWidget->setCurrentWidget(page);
    page->setFocus();
    qDebug() << "New Tab:" << ID << dirs[i];
    connect(page, SIGNAL(ProcessClosed(QString)), this, SLOT(Close_Tab(QString)) );
  }
}

void TermWindow::setCurrentScreen(int num){
    screennum = num;
    QTimer::singleShot(0,this, SLOT(ReShowWindow()));
}

void TermWindow::setTopOfScreen(bool ontop){
    onTop = ontop;
    this->layout()->setContentsMargins(0, (onTop ? 0 : 3), 0, (onTop ? 3 : 0));
    tabWidget->setTabPosition(onTop ? QTabWidget::South : QTabWidget::North);
    QTimer::singleShot(0,this, SLOT(ReShowWindow()));
}

// =======================
//       PUBLIC SLOTS
// =======================
void TermWindow::ShowWindow(){
  if(animRunning>=0){ return; } //something running
  animRunning = 1;
  this->hide();
  QApplication::processEvents();
  CalculateGeom();
  //Now setup the animation
  ANIM->setEndValue(this->geometry());
  if(onTop){ //use top edge
    ANIM->setStartValue( QRect(this->x(), this->y(), this->width(), 0) ); //same location - no height
  }else{
    ANIM->setStartValue( QRect(this->x(), this->geometry().bottom(), this->width(), 0) ); //same location - no height
  }
  this->show();
  //qDebug() << "Start Animation" << ANIM->startValue() << ANIM->endValue();
  ANIM->start();
}

void TermWindow::HideWindow(){
  if(animRunning>=0){ return; } //something running
  //Now setup the animation
  //Note: Do *not* use the private settings/variables because it may be changing right now - use the current geometry *ONLY*
  animRunning = 0;
  ANIM->setStartValue(this->geometry());
  QDesktopWidget *desk = QApplication::desktop();
  int screen = desk->screenNumber(this); //which screen it is currently on
  if(desk->availableGeometry(screen).top() == this->geometry().top()){ //use top edge
    ANIM->setEndValue( QRect(this->x(), this->y(), this->width(), 0) ); //same location - no height
  }else{
    ANIM->setEndValue( QRect(this->x(), this->y()+this->height(), this->width(), 0) ); //same location - no height
  }
  this->show();
  ANIM->start();	
}

void TermWindow::CloseWindow(){
  if(animRunning>=0){ return; } //something running
  //Now setup the animation
  animRunning = 2;
  ANIM->setStartValue(this->geometry());
  if(onTop){ //use top edge
    ANIM->setEndValue( QRect(this->x(), this->y(), this->width(), 0) ); //same location - no height
  }else{
    ANIM->setEndValue( QRect(this->x(), this->geometry().bottom(), this->width(), 0) ); //same location - no height
  }
  this->show();
  ANIM->start();		
}

void TermWindow::ReShowWindow(){
  if(this->isVisible()){
    HideWindow(); //start with same animation as hide
    animRunning = 3; //flag as a re-show (hide, then show);
  }else{
    //Already hidden, just show it
    ShowWindow();
  }
}
// =======================
//             PRIVATE
// =======================
void TermWindow::CalculateGeom(){
  //qDebug() << "Calculating Geom:" << this->size();
  QDesktopWidget *desk = QApplication::desktop();
  if(desk->screenCount() <= screennum){ screennum = desk->primaryScreen(); } //invalid screen detected 
  //Now align the window with the proper screen edge
  QRect workarea = desk->availableGeometry(screennum); //this respects the WORKAREA property
  if(onTop){
    this->setGeometry( workarea.x(), workarea.y(), workarea.width(), this->height()); //maintain current hight of window
	  
  }else{
    this->setGeometry( workarea.x(), workarea.y() + workarea.height() - this->height(), workarea.width(), this->height()); //maintain current hight of window
  }
  this->setFixedWidth(this->width()); //Make sure the window is not re-sizeable in the width dimension
  this->setMinimumHeight(0);
}

QString TermWindow::GenerateTabID(){
  //generate a unique ID for this new tab
  int num = 1;
  for(int i=0; i<tabWidget->count(); i++){
    if(tabWidget->widget(i)->whatsThis().toInt() >= num){ num = tabWidget->widget(i)->whatsThis().toInt()+1; }
  }
  return QString::number(num);
}

// =======================
//        PRIVATE  SLOTS
// =======================

//Tab Interactions
void TermWindow::New_Tab(){
  OpenDirs(QStringList() << QDir::homePath());
}

void TermWindow::Close_Tab(int tab){
  qDebug() << "Close Tab:" << tab;
  if(tab<0){ tab = tabWidget->currentIndex(); }
  static_cast<TerminalWidget*>(tabWidget->widget(tab))->aboutToClose();
  tabWidget->widget(tab)->deleteLater(); //delete the page within the tag
  tabWidget->removeTab(tab); // remove the tab itself
  //Let the tray know when the last terminal is closed
  if(tabWidget->count() < 1){ 
    emit TerminalFinished();
  }
}

void TermWindow::Close_Tab(QString ID){
  //Close a tab based on it's ID instead of it's tab number
  qDebug() << "Close Tab by ID:" << ID;
  for(int i=0; i<tabWidget->count(); i++){
    if(tabWidget->widget(i)->whatsThis()==ID){
      qDebug() << " - Start close by number:" << i;
      Close_Tab(i);
      return; //all done
    }
  }
}

void TermWindow::Next_Tab(){
  qDebug() << "Next Tab";
  int next = tabWidget->currentIndex()+1;
  if(next>=tabWidget->count()){ next = 0; }
  tabWidget->setCurrentIndex(next);
}

void TermWindow::Prev_Tab(){
  qDebug() << "Previous Tab";
  int next = tabWidget->currentIndex()-1;
  if(next<0){ next = tabWidget->count()-1; }
  tabWidget->setCurrentIndex(next);	
}

void TermWindow::focusOnWidget(){
  if(tabWidget->currentWidget()!=0){
    tabWidget->currentWidget()->setFocus();
  }
}

//Animation finishing
void TermWindow::AnimFinished(){
  if(animRunning <0){ return; } //nothing running
  if(animRunning==0){
    //Hide Event
    this->hide(); //need to hide the whole thing now
    this->setGeometry( ANIM->startValue().toRect() ); //reset back to initial size after hidden
    emit TerminalHidden();
  }else if(animRunning==1){
    //Show Event
    this->activateWindow();
    tabWidget->currentWidget()->setFocus();
    emit TerminalVisible();
  }else if(animRunning==2){
    //Close Event
    this->hide(); //need to hide the whole thing now  
    emit TerminalClosed();
  }else if(animRunning>2){
    //Re-Show event
    this->hide();
    this->setGeometry( ANIM->startValue().toRect() ); //reset back to initial size after hidden
    //Now re-show it
    QTimer::singleShot(0,this, SLOT(ShowWindow()));
  }
  animRunning = -1; //done
}
 
void TermWindow::activeStatusChanged(){
  if(animRunning>=0){ return; } //ignore this event - already changing
  QWidget *active = QApplication::activeWindow();
  if(active==0 && this->isVisible()){ HideWindow(); }
}

// ===================
//        PROTECTED
// ===================
void TermWindow::mouseMoveEvent(QMouseEvent *ev){
  //Note: With mouse tracking turned off, this event only happens when the user is holding down the mouse button
    if(onTop){
      //Move the bottom edge to the current point
      if( (ev->globalPos().y() - this->y()) < 50){ return; } //quick check that the window is not smaller than 20 pixels
      QRect geom = this->geometry();
	    geom.setBottom(ev->globalPos().y());
      this->setGeometry(geom);
    }else{
      //Move the top edge to the current point
      if( (this->y() + this->height() -ev->globalPos().y()) < 50){ return; } //quick check that the window is not smaller than 20 pixels
      QRect geom = this->geometry();
	    geom.setTop(ev->globalPos().y());
      this->setGeometry(geom);
    }
    settings->setValue("lastSize",this->geometry().size());
}
