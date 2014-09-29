//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "UserItemWidget.h"

UserItemWidget::UserItemWidget(QWidget *parent, QString itemPath, bool isDir) : QFrame(parent){
  createWidget();
  //Now fill it appropriately
  if(itemPath.endsWith(".desktop")){
    bool ok = false;
    XDGDesktop item = LXDG::loadDesktopFile(itemPath, ok);
    if(ok){
      icon->setPixmap( LXDG::findIcon(item.icon, "preferences-system-windows-actions").pixmap(30,30) );
      name->setText( item.name );
    }else{
      icon->setPixmap( LXDG::findIcon("unknown","").pixmap(30,30) );
      name->setText( itemPath.section("/",-1) );
    }
  }else if(isDir){
    if(itemPath.endsWith("/")){ itemPath.chop(1); }
    icon->setPixmap( LXDG::findIcon("folder","").pixmap(30,30) );
    name->setText( itemPath.section("/",-1) );	  
  }else{
    if(itemPath.endsWith("/")){ itemPath.chop(1); }
    icon->setPixmap( LXDG::findMimeIcon(itemPath.section("/",-1).section(".",-1)).pixmap(30,30) );
    name->setText( itemPath.section("/",-1) );
  }
  linkPath = QFile::symLinkTarget(itemPath);
  icon->setWhatsThis(itemPath);
  if(isDir && !linkPath.isEmpty()){ isDir = false; } //not a real directory - just a sym link
  isDirectory = isDir; //save this for later
  isShortcut = itemPath.contains("/home/") && (itemPath.contains("/Desktop/") || itemPath.contains("/.lumina/favorites/") );
  //Now setup the button appropriately
  setupButton();
}

UserItemWidget::UserItemWidget(QWidget *parent, XDGDesktop item) : QFrame(parent){
  createWidget();
  isDirectory = false;
  linkPath = QFile::symLinkTarget(item.filePath);
  isShortcut = item.filePath.contains("/home/") && (item.filePath.contains("/Desktop/") || item.filePath.contains("/.lumina/favorites/") );
  //Now fill it appropriately
  icon->setPixmap( LXDG::findIcon(item.icon,"preferences-system-windows-actions").pixmap(30,30) );
  name->setText( item.name );
  icon->setWhatsThis(item.filePath);
  //Now setup the button appropriately
  setupButton();

}

UserItemWidget::~UserItemWidget(){ 
  delete button;
  delete icon;
  delete name;
}


void UserItemWidget::createWidget(){
  //Initialize the widgets
  this->setContentsMargins(0,0,0,0);
  button = new QToolButton(this);
    button->setIconSize( QSize(14,14) );
    button->setAutoRaise(true);
  icon = new QLabel(this);
    icon->setFixedSize( QSize(30,30) );
  name = new QLabel(this);
  //Add them to the layout
  this->setLayout(new QHBoxLayout());
    this->layout()->setContentsMargins(1,1,1,1);
    this->layout()->addWidget(icon);
    this->layout()->addWidget(name);
    this->layout()->addWidget(button);
  //Install the stylesheet
  this->setStyleSheet("UserItemWidget{ background: transparent; border-radius: 5px;} UserItemWidget::hover{ background: rgba(255,255,255,200); border-radius: 5px; }");
}

void UserItemWidget::setupButton(){
  if( !isDirectory && isShortcut ){
    //This is a current desktop shortcut -- allow the user to remove it
    button->setWhatsThis("remove");
    if(!linkPath.isEmpty()){
      button->setIcon( LXDG::findIcon("list-remove","") );
      button->setToolTip(tr("Remove Shortcut"));
    }else{
      button->setIcon( LXDG::findIcon("user-trash","") );
      button->setToolTip(tr("Delete File"));
    }
    connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()) );
  }else if( !QFile::exists( QDir::homePath()+"/Desktop/"+icon->whatsThis().section("/",-1) ) && !QFile::exists( QDir::homePath()+"/.lumina/favorites/"+icon->whatsThis().section("/",-1) ) ){
    //This file does not have a desktop shortcut yet -- allow the user to add it
    button->setWhatsThis("add");
    button->setIcon( LXDG::findIcon("favorites","") );    
    button->setToolTip(tr("Create Shortcut"));
    connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()) );
  }else{
    //This already has a desktop shortcut -- no special actions
    button->setVisible(false);
  }
}

void UserItemWidget::buttonClicked(){
  button->setVisible(false);
  if(button->whatsThis()=="add"){ 
    QFile::link(icon->whatsThis(), QDir::homePath()+"/.lumina/favorites/"+icon->whatsThis().section("/",-1) );
    emit NewShortcut(); 
  }else if(button->whatsThis()=="remove"){ 
    QFile::remove(icon->whatsThis()); //never remove the linkPath - since that is the actual file/dir
    emit RemovedShortcut(); 
  }
}

void UserItemWidget::ItemClicked(){
  if(!linkPath.isEmpty()){ emit RunItem("lumina-open \""+linkPath+"\"" ); }
  else{ emit RunItem("lumina-open \""+icon->whatsThis()+"\"" ); }
  
}