//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "TrayIcon.h"

#include <QDir>
#include <QDesktopWidget>

#include <LUtils.h>

TrayIcon::TrayIcon() : QSystemTrayIcon(){
  //Create the child widgets here
  settings = new QSettings("lumina-desktop","lumina-terminal");
  this->setContextMenu(new QMenu());
  ScreenMenu = new QMenu();
    connect(ScreenMenu, SIGNAL(triggered(QAction*)), this, SLOT(ChangeScreen(QAction*)) );
  TERM = new TermWindow(settings);
    //Load the current settings
    TERM->setTopOfScreen(settings->value("TopOfScreen",true).toBool());
    TERM->setCurrentScreen(settings->value("OnScreen",0).toInt());
  connect(TERM, SIGNAL(TerminalHidden()), this, SLOT(TermHidden()));
  connect(TERM, SIGNAL(TerminalVisible()), this, SLOT(TermVisible()));
  connect(TERM, SIGNAL(TerminalClosed()), this, SLOT(startCleanup()));
  connect(TERM, SIGNAL(TerminalFinished()), this, SLOT(stopApplication()));
  connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(TrayActivated(QSystemTrayIcon::ActivationReason)) );
}

TrayIcon::~TrayIcon(){
  delete TERM;
  delete ScreenMenu;
}

// =============
//      PUBLIC
// =============
void TrayIcon::parseInputs(QStringList inputs){
  //Note that this is only run on the primary process - otherwise inputs are sent to the slotSingleInstance() below
  termVisible = !inputs.contains("-toggle"); //will automatically show the terminal on first run, even if "-toggle" is set

  setupContextMenu();
  updateIcons();
  inputs = adjustInputs(inputs); //will adjust termVisible as necessary
  if(inputs.isEmpty()){ inputs << QDir::homePath(); } //always start up with one terminal minimum
  TERM->OpenDirs(inputs);
  if(termVisible){ QTimer::singleShot(0, TERM, SLOT(ShowWindow())); }
}
	   
// =================
//     PUBLIC SLOTS
// =================
void TrayIcon::slotSingleInstance(QStringList inputs){
  //Note that this is only run for a secondary process forwarding its inputs
  //qDebug() << "Single Instance Event:" << inputs << termVisible;	
  bool visible = termVisible;
  inputs = adjustInputs(inputs); //will adjust termVisible as necessary
  if(!inputs.isEmpty()){ TERM->OpenDirs(inputs); }
  //Only adjust the window if there was a change in the visibility status
  //qDebug() << "Set Visible:" << termVisible;
  if(!visible && termVisible){ QTimer::singleShot(0, TERM, SLOT(ShowWindow())); }
  else if(visible && !termVisible){ QTimer::singleShot(0, TERM, SLOT(HideWindow())); }
}

void TrayIcon::updateIcons(){
  this->setIcon(LXDG::findIcon("utilities-terminal",""));
}

// ================
//         PRIVATE
// ================
QStringList TrayIcon::adjustInputs(QStringList inputs){
  bool hasHide = false;
  //Look for the special CLI flags just for the tray icon and trim them out
  for(int i=0; i<inputs.length(); i++){
    if(inputs[i]=="-toggle"){ hasHide = termVisible; inputs.removeAt(i); i--; } //toggle the visibility
    else if(inputs[i]=="-show"){ hasHide = false; inputs.removeAt(i); i--; } //change the visibility
    else if(inputs[i]=="-hide"){  hasHide = true; inputs.removeAt(i); i--; } //change the visibility
    else{
	//Must be a directory - convert to an absolute path and check for existance
	inputs[i] = LUtils::PathToAbsolute(inputs[i]);
	QFileInfo info(inputs[i]);
	if(!info.exists()){
	  qDebug() << "Directory does not exist: " << inputs[i];
	  inputs.removeAt(i);
	  i--;
	}else if(!info.isDir()){
	 //Must be some kind of file, open the parent directory
	  inputs[i] = inputs[i].section("/",0,-2);
	}
    }
  }	  
  termVisible = !hasHide;
  return inputs;
}

// ================
//  PRIVATE SLOTS
// ================
void TrayIcon::startCleanup(){
  TERM->cleanup();	
}

void TrayIcon::stopApplication(){
  QApplication::exit(0); 
}

void TrayIcon::ChangeTopBottom(bool ontop){
  TERM->setTopOfScreen(ontop);	
  settings->setValue("TopOfScreen",ontop); //save for later
}

void TrayIcon::ChangeScreen(QAction *act){
  int screen = act->whatsThis().toInt();
  TERM->setCurrentScreen(screen);
  settings->setValue("OnScreen",screen);
  updateScreenMenu();
}

void TrayIcon::setupContextMenu(){
  this->contextMenu()->clear();
  this->contextMenu()->addAction(LXDG::findIcon("edit-select",""), tr("Trigger Terminal"), this, SLOT(ToggleVisibility()) );
  this->contextMenu()->addSeparator();
  QAction * act = this->contextMenu()->addAction(tr("Top of Screen"), this, SLOT(ChangeTopBottom(bool)) );
    act->setCheckable(true);
    act->setChecked(settings->value("TopOfScreen",true).toBool() );
  this->contextMenu()->addMenu(ScreenMenu);
  this->contextMenu()->addSeparator();
  this->contextMenu()->addAction(LXDG::findIcon("application-exit",""), tr("Close Terminal"), this, SLOT(stopApplication()) );
  updateScreenMenu();
}

void TrayIcon::updateScreenMenu(){
  ScreenMenu->clear();
  QDesktopWidget *desk = QApplication::desktop();
  int cscreen = settings->value("OnScreen",0).toInt();
  if(cscreen>=desk->screenCount()){ cscreen = desk->primaryScreen(); }
  ScreenMenu->setTitle(tr("Move To Monitor"));
  for(int i=0; i<desk->screenCount(); i++){
    if(i!=cscreen){
      QAction *act = new QAction( QString(tr("Monitor %1")).arg(QString::number(i+1)),ScreenMenu);
	act->setWhatsThis(QString::number(i));
      ScreenMenu->addAction(act);
    }
  }
  ScreenMenu->setVisible(!ScreenMenu->isEmpty());
  ScreenMenu->setEnabled(!ScreenMenu->isEmpty());
}

void TrayIcon::TrayActivated(QSystemTrayIcon::ActivationReason reason){
  switch(reason){
    case QSystemTrayIcon::Context:
	this->contextMenu()->popup(this->geometry().center());
        break;
    default:
	ToggleVisibility();
  }
}

//Slots for the window visibility
void TrayIcon::ToggleVisibility(){
  if(termVisible){ QTimer::singleShot(0, TERM, SLOT(HideWindow())); }
  else{ QTimer::singleShot(0, TERM, SLOT(ShowWindow())); }
}

void TrayIcon::TermHidden(){
  termVisible = false;
}

void TrayIcon::TermVisible(){
  termVisible = true;
}
