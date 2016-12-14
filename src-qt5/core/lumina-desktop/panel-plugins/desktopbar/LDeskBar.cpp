//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LDeskBar.h"
#include "../../LSession.h"

LDeskBarPlugin::LDeskBarPlugin(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  this->layout()->setContentsMargins(0,0,0,0);
  this->setStyleSheet( "QToolButton::menu-indicator{ image: none; } QToolButton{ padding: 0px; }");

  //initialize the desktop bar items
  initializeDesktop();
  //setup the directory watcher
  QString fav = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/favorites.list";
  if(!QFile::exists(fav)){ QProcess::execute("touch \""+fav+"\""); }
  watcher = new QFileSystemWatcher(this);
    watcher->addPath( fav );
  connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(filechanged(QString)) );
  QTimer::singleShot(1,this, SLOT(updateFiles()) ); //make sure to load it the first time
  QTimer::singleShot(0,this, SLOT(OrientationChange()) ); //adjust sizes/layout
  connect(QApplication::instance(), SIGNAL(DesktopFilesChanged()), this, SLOT(updateFiles()) );
}

LDeskBarPlugin::~LDeskBarPlugin(){
}

// =======================
//   PRIVATE FUNCTIONS
// =======================
void LDeskBarPlugin::initializeDesktop(){
  //Applications on the desktop
  appB = new QToolButton(this);
    appB->setToolButtonStyle(Qt::ToolButtonIconOnly);
    appB->setAutoRaise(true);
    appB->setPopupMode(QToolButton::InstantPopup);
  appM = new QMenu(this);
    appB->setMenu(appM);
    this->layout()->addWidget(appB);
    connect(appM,SIGNAL(triggered(QAction*)),this,SLOT(ActionTriggered(QAction*)) );
    connect(appM, SIGNAL(aboutToHide()), this, SIGNAL(MenuClosed()));
  //Directories on the desktop
  dirB = new QToolButton(this);
    dirB->setToolButtonStyle(Qt::ToolButtonIconOnly);
    dirB->setAutoRaise(true);
    dirB->setPopupMode(QToolButton::InstantPopup);
  dirM = new QMenu(this);
    dirB->setMenu(dirM);
    this->layout()->addWidget(dirB);
    connect(dirM,SIGNAL(triggered(QAction*)),this,SLOT(ActionTriggered(QAction*)) );
    connect(dirM, SIGNAL(aboutToHide()), this, SIGNAL(MenuClosed()));
  //Audio Files on the desktop
  audioM = new QMenu(this);
    connect(audioM,SIGNAL(triggered(QAction*)),this,SLOT(ActionTriggered(QAction*)) );
  //Video Files on the desktop
  videoM = new QMenu(this);
    connect(videoM,SIGNAL(triggered(QAction*)),this,SLOT(ActionTriggered(QAction*)) );
  //Picture Files on the desktop
  pictureM = new QMenu(this);
    connect(pictureM,SIGNAL(triggered(QAction*)),this,SLOT(ActionTriggered(QAction*)) );
  //Other Files on the desktop
  otherM = new QMenu(this);
    connect(otherM,SIGNAL(triggered(QAction*)),this,SLOT(ActionTriggered(QAction*)) );
  docM = new QMenu(this);
    connect(docM,SIGNAL(triggered(QAction*)), this,SLOT(ActionTriggered(QAction*)) );
  //All Files Button
  fileB = new QToolButton(this);
    fileB->setToolButtonStyle(Qt::ToolButtonIconOnly);
    fileB->setAutoRaise(true);
    fileB->setPopupMode(QToolButton::InstantPopup);
  fileM = new QMenu(this);
    fileB->setMenu(fileM);
    this->layout()->addWidget(fileB);
    
    updateIcons(); //set all the text/icons
}

QAction* LDeskBarPlugin::newAction(QString filepath, QString name, QString iconpath){
  return newAction(filepath, name, QIcon(iconpath));
}

QAction* LDeskBarPlugin::newAction(QString filepath, QString name, QIcon icon){
  QAction *act = new QAction(icon, name, this);
    act->setWhatsThis(filepath);
  return act;	 
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
void LDeskBarPlugin::filechanged(QString file){
  updateFiles();
  if(!watcher->files().contains(file)){ watcher->addPath(file); } //make sure the file does not get removed from the watcher
}
void LDeskBarPlugin::updateFiles(){
  QFileInfoList homefiles = LSession::handle()->DesktopFiles();
  QStringList favitems = LDesktopUtils::listFavorites();
  //Remember for format for favorites: <name>::::[app/dir/<mimetype>]::::<full path>
    for(int i=0; i<homefiles.length(); i++){
      if( !favitems.filter(homefiles[i].canonicalFilePath()).isEmpty() ){ continue; } //duplicate entry
      QString type;
      if(homefiles[i].isDir()){ type="dir"; }
      else if(homefiles[i].fileName().endsWith(".desktop")){ type="app"; }
      else{ type=LXDG::findAppMimeForFile(homefiles[i].fileName()); }
      favitems << homefiles[i].fileName()+"::::"+type+"::::"+homefiles[i].absoluteFilePath();
      //qDebug() << "Desktop Item:" << favitems.last();
    }
  
  favitems.sort(); //sort them alphabetically
  //Now add the items to the lists
  appM->clear();
  dirM->clear();
  audioM->clear();
  videoM->clear();
  pictureM->clear();
  docM->clear();
  otherM->clear();
  for(int i=0; i<favitems.length(); i++){
    QString type = favitems[i].section("::::",1,1);
    QString name = favitems[i].section("::::",0,0);
    QString path = favitems[i].section("::::",2,50);
    if(type=="app"){
      //Add it to appM
      bool ok = false;
      XDGDesktop df(path);
      if(df.isValid() && !df.isHidden){
	  appM->addAction( newAction(df.filePath, df.name, LXDG::findIcon(df.icon, ":/images/default-application.png")) );
      }
    }else if(type=="dir"){
      //Add it to dirM
      dirM->addAction( newAction(path, name, LXDG::findIcon("folder","")) );
    }else if(type.startsWith("audio/")){
      //Add it to audioM
      audioM->addAction( newAction(path, name, LXDG::findMimeIcon(type)) );
    }else if(type.startsWith("video/")){
      //Add it to videoM
      videoM->addAction( newAction(path, name, LXDG::findMimeIcon(type)) );
    }else if(type.startsWith("image/")){
      //Add it to pictureM
      if(LUtils::imageExtensions().contains(path.section("/",-1).section(".",-1).toLower()) ){
	pictureM->addAction( newAction(path, name, QIcon(path)) );
      }else{
        pictureM->addAction( newAction(path, name, LXDG::findMimeIcon(type)) );
      }
    }else if(type.startsWith("text/")){
      //Add it to docM
      docM->addAction( newAction(path, name, LXDG::findMimeIcon(type)) );
    }else{
      //Add it to otherM
      otherM->addAction( newAction(path, name, LXDG::findMimeIcon(type)) );
    }

  }

    //Now update the file menu as appropriate
    fileM->clear();
    if(!audioM->isEmpty()){ fileM->addMenu(audioM); }
    if(!docM->isEmpty()){ fileM->addMenu(docM); }
    if(!pictureM->isEmpty()){ fileM->addMenu(pictureM); }
    if(!videoM->isEmpty()){ fileM->addMenu(videoM); }
    if(!otherM->isEmpty()){ fileM->addMenu(otherM); }
    //Check for a single submenu, and skip the main if need be
    disconnect(fileB->menu(), SIGNAL(aboutToHide()), this, SIGNAL(MenuClosed()) );
    if(fileM->actions().length()==1){
      if(!audioM->isEmpty()){ fileB->setMenu(audioM); }
      else if(!pictureM->isEmpty()){ fileB->setMenu(pictureM); }
      else if(!videoM->isEmpty()){ fileB->setMenu(videoM); }
      else if(!docM->isEmpty()){ fileB->setMenu(docM); }
      else if(!otherM->isEmpty()){ fileB->setMenu(otherM); }
    }else{
      fileB->setMenu(fileM);	    
    }
    connect(fileB->menu(), SIGNAL(aboutToHide()), this, SIGNAL(MenuClosed()));

  //Setup the visibility of the buttons
  appB->setVisible( !appM->isEmpty() );
  dirB->setVisible( !dirM->isEmpty() );
  fileB->setVisible( !fileM->isEmpty() );
}
	
void LDeskBarPlugin::updateIcons(){
  //Set all the text/icons
    appB->setIcon( LXDG::findIcon("favorites", "") );
    appB->setToolTip(tr("Favorite Applications"));
    dirB->setIcon( LXDG::findIcon("folder", "") );
    dirB->setToolTip(tr("Favorite Folders"));
    audioM->setTitle( tr("Audio") );
    audioM->setIcon( LXDG::findIcon("audio-x-generic","") );
    videoM->setTitle( tr("Video") );
    videoM->setIcon( LXDG::findIcon("video-x-generic","") );
    pictureM->setTitle( tr("Pictures") );
    pictureM->setIcon( LXDG::findIcon("image-x-generic","") );
    otherM->setTitle( tr("Other Files") );
    otherM->setIcon( LXDG::findIcon("unknown","") );
    docM->setTitle( tr("Documents") );
    docM->setIcon( LXDG::findIcon("x-office-document","") );
    fileB->setIcon( LXDG::findIcon("document-multiple", "") );
    fileB->setToolTip(tr("Favorite Files") );
}
