//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LDeskBar.h"
#include "../../LSession.h"

LDeskBarPlugin::LDeskBarPlugin(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  this->layout()->setContentsMargins(0,0,0,0);
  this->setStyleSheet( "QToolButton::menu-indicator{ image: none; } QToolButton{ padding: 0px; }");
  //Find the path to the desktop folder
  if(QFile::exists(QDir::homePath()+"/Desktop")){ desktopPath = QDir::homePath()+"/Desktop"; }
  else if(QFile::exists(QDir::homePath()+"/desktop")){ desktopPath = QDir::homePath()+"/desktop"; }
  else{ desktopPath=""; }
  //Setup the filter lists for the different types of files
  audioFilter <<"*.ogg"<<"*.mp3"<<"*.wav"<<"*.aif"<<"*.iff"<<"*.m3u"<<"*.m4a"<<"*.mid"<<"*.mpa"<<"*.ra"<<"*.wma";
  videoFilter <<"*.3g2"<<"*.3gp"<<"*.asf"<<"*.asx"<<"*.avi"<<"*.flv"<<"*.m4v"<<"*.mov"<<"*.mp4"<<"*.mpg"<<"*.rm"<<"*.srt"<<"*.swf"<<"*.vob"<<"*.wmv";
  pictureFilter <<"*.bmp"<<"*.dds"<<"*.gif"<<"*.jpg"<<"*.png"<<"*.psd"<<"*.thm"<<"*.tif"<<"*.tiff"<<"*.ai"<<"*.eps"<<"*.ps"<<"*.svg"<<"*.ico";
  docsFilter << "*.txt"<<"*.rtf"<<"*.doc"<<"*.docx"<<"*.odf"<<"*.pdf";
  //initialize the desktop bar items
  initializeDesktop();
  //setup the directory watcher
  watcher = new QFileSystemWatcher(this);
  if(!desktopPath.isEmpty()){ 
    watcher->addPath(desktopPath); 
  }
  connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(desktopChanged()) );
  QTimer::singleShot(1,this, SLOT(desktopChanged()) ); //make sure to load it the first time
  QTimer::singleShot(0,this, SLOT(OrientationChange()) ); //adjust sizes/layout
}

LDeskBarPlugin::~LDeskBarPlugin(){
  if(!desktopPath.isEmpty()){
    watcher->removePath(desktopPath);
    disconnect(watcher);
  }
  delete watcher;
  
}

// =======================
//   PRIVATE FUNCTIONS
// =======================
void LDeskBarPlugin::initializeDesktop(){
  //Applications on the desktop
  appB = new QToolButton(this);
    appB->setIcon( LXDG::findIcon("favorites", "") );
    appB->setToolButtonStyle(Qt::ToolButtonIconOnly);
    appB->setToolTip(tr("Favorite Applications"));
    appB->setAutoRaise(true);
    appB->setPopupMode(QToolButton::InstantPopup);
  appM = new QMenu(this);
    appB->setMenu(appM);
    this->layout()->addWidget(appB);
    connect(appM,SIGNAL(triggered(QAction*)),this,SLOT(ActionTriggered(QAction*)) );
  //Directories on the desktop
  dirB = new QToolButton(this);
    dirB->setIcon( LXDG::findIcon("folder", "") );
    dirB->setToolButtonStyle(Qt::ToolButtonIconOnly);
    dirB->setToolTip(tr("Favorite Folders"));
    dirB->setAutoRaise(true);
    dirB->setPopupMode(QToolButton::InstantPopup);
  dirM = new QMenu(this);
    dirB->setMenu(dirM);
    this->layout()->addWidget(dirB);
    connect(dirM,SIGNAL(triggered(QAction*)),this,SLOT(ActionTriggered(QAction*)) );
  //Audio Files on the desktop
  audioM = new QMenu(tr("Audio"), this);
    connect(audioM,SIGNAL(triggered(QAction*)),this,SLOT(ActionTriggered(QAction*)) );
    audioM->setIcon( LXDG::findIcon("audio-x-generic","") );
  //Video Files on the desktop
  videoM = new QMenu(tr("Video"), this);
    connect(videoM,SIGNAL(triggered(QAction*)),this,SLOT(ActionTriggered(QAction*)) );
    videoM->setIcon( LXDG::findIcon("video-x-generic","") );
  //Picture Files on the desktop
  pictureM = new QMenu(tr("Pictures"), this);
    connect(pictureM,SIGNAL(triggered(QAction*)),this,SLOT(ActionTriggered(QAction*)) );
    pictureM->setIcon( LXDG::findIcon("image-x-generic","") );
  //Other Files on the desktop
  otherM = new QMenu(tr("Other Files"), this);
    connect(otherM,SIGNAL(triggered(QAction*)),this,SLOT(ActionTriggered(QAction*)) );
    otherM->setIcon( LXDG::findIcon("unknown","") );
  docM = new QMenu(tr("Documents"), this);
    connect(docM,SIGNAL(triggered(QAction*)), this,SLOT(ActionTriggered(QAction*)) );
    docM->setIcon( LXDG::findIcon("x-office-document","") );
  //All Files Button
  fileB = new QToolButton(this);
    fileB->setIcon( LXDG::findIcon("document-multiple", "") );
    fileB->setToolButtonStyle(Qt::ToolButtonIconOnly);
    fileB->setToolTip(tr("Favorite Files") );
    fileB->setAutoRaise(true);
    fileB->setPopupMode(QToolButton::InstantPopup);
  fileM = new QMenu(this);
    fileB->setMenu(fileM);
    this->layout()->addWidget(fileB);
}

QAction* LDeskBarPlugin::newAction(QString filepath, QString name, QString iconpath){
  return newAction(filepath, name, QIcon(iconpath));
}

QAction* LDeskBarPlugin::newAction(QString filepath, QString name, QIcon icon){
  QAction *act = new QAction(icon, name, this);
    act->setWhatsThis(filepath);
  return act;	 
}

void LDeskBarPlugin::updateMenu(QMenu* menu, QFileInfoList files, bool trim){
  menu->clear();
  //re-create the menu (since it is hidden from view)
  for(int i=0; i<files.length(); i++){
    qDebug() << "New Menu Item:" << files[i].fileName();
    if(trim){ totals.removeAll(files[i]); }
    menu->addAction( newAction( files[i].canonicalFilePath(), files[i].fileName(), "") );
  }
}

// =======================
//     PRIVATE SLOTS
// =======================
void LDeskBarPlugin::ActionTriggered(QAction* act){
 //Open up the file with the appropriate application
 QString cmd = "lumina-open \""+act->whatsThis()+"\"";
 qDebug() << "Open File:" << cmd;
 LSession::LaunchApplication(cmd);
}

void LDeskBarPlugin::desktopChanged(){
  if(!desktopPath.isEmpty()){
    QDir dir(desktopPath);
    totals = dir.entryInfoList( QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    //Update all the special menus (trimming the totals list as we go)
    updateMenu(dirM, dir.entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name) );
    updateMenu(audioM, dir.entryInfoList( audioFilter, QDir::Files, QDir::Name) );
    updateMenu(videoM, dir.entryInfoList( videoFilter, QDir::Files, QDir::Name) );
    updateMenu(pictureM, dir.entryInfoList( pictureFilter, QDir::Files, QDir::Name) );
    updateMenu(docM, dir.entryInfoList( docsFilter, QDir::Files, QDir::Name) );
    //Now update the launchers
    QFileInfoList exe = dir.entryInfoList( QStringList() << "*.desktop", QDir::Files, QDir::Name );
      // - Get a complete list of apps (in alphabetical order)
      QList<XDGDesktop> exeList;
      for(int i=0; i<exe.length(); i++){
      	totals.removeAll(exe[i]); //Remove this item from the totals
      	bool ok = false;
        XDGDesktop df = LXDG::loadDesktopFile(exe[i].canonicalFilePath(), ok);
        if(ok){
          if( LXDG::checkValidity(df) && !df.isHidden ){ exeList << df; }
        }
      }
      exeList = LXDG::sortDesktopNames(exeList);
      // - Now re-create the menu with the apps
      appM->clear();
      bool listApps = true; //turn this off for the moment (make dynamic later)
      if(!listApps){
	//go through the current items and remove them all
	while( APPLIST.length() > 0){
	  delete this->layout()->takeAt(3); //always after the 3 main menu buttons
	}
      }
      for(int i=0; i<exeList.length(); i++){
        if(listApps){ appM->addAction( newAction(exeList[i].filePath, exeList[i].name, LXDG::findIcon(exeList[i].icon, ":/images/default-application.png")) ); }
	else{
	  //Create a new LTBWidget for this app
	  QToolButton *it = new QToolButton(this);
		it->setWhatsThis(exeList[i].filePath);
		it->setToolTip(exeList[i].name);
		it->setIcon( LXDG::findIcon(exeList[i].icon, "") );
		it->setToolButtonStyle(Qt::ToolButtonIconOnly);
	        it->setAutoRaise(true);
		it->setPopupMode(QToolButton::InstantPopup);
		if(it->icon().isNull()){ it->setIcon( LXDG::findIcon("application-x-executable","") ); }
		connect(it, SIGNAL(triggered(QAction*)), this , SLOT(ActionTriggered(QAction*)) );
	  APPLIST << it;
	  this->layout()->addWidget(it);
	}
      }
    //Now update the other menu with everything else that is left
    updateMenu(otherM, totals, false);
    //Now update the file menu as appropriate
    fileM->clear();
    if(!audioM->isEmpty()){ fileM->addMenu(audioM); }
    if(!docM->isEmpty()){ fileM->addMenu(docM); }
    if(!pictureM->isEmpty()){ fileM->addMenu(pictureM); }
    if(!videoM->isEmpty()){ fileM->addMenu(videoM); }
    if(!otherM->isEmpty()){ fileM->addMenu(otherM); }
    //Check for a single submenu, and skip the main if need be
    if(fileM->actions().length()==1){
      if(!audioM->isEmpty()){ fileB->setMenu(audioM); }
      else if(!pictureM->isEmpty()){ fileB->setMenu(pictureM); }
      else if(!videoM->isEmpty()){ fileB->setMenu(videoM); }
      else if(!docM->isEmpty()){ fileB->setMenu(docM); }
      else if(!otherM->isEmpty()){ fileB->setMenu(otherM); }
    }else{
      fileB->setMenu(fileM);	    
    }
  }	
  //Setup the visibility of the buttons
  appB->setVisible( !appM->isEmpty() );
  dirB->setVisible( !dirM->isEmpty() );
  fileB->setVisible( !fileM->isEmpty() );
  //Clear the totals list (since no longer in use)
  totals.clear();
}
	
