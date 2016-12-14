//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "ItemWidget.h"
#include <LUtils.h>
#include <QMenu>
#include "../../LSession.h"


ItemWidget::ItemWidget(QWidget *parent, QString itemPath, QString type, bool goback) : QFrame(parent){
  createWidget();
  //Now fill it appropriately
  bool inHome = type.endsWith("-home"); //internal code
  if(inHome){ type = type.remove("-home"); }
  if(itemPath.endsWith(".desktop") || type=="app"){
    XDGDesktop item(itemPath, this);
    gooditem = item.isValid();
    //qDebug() << "Good Item:" << gooditem << itemPath;
    if(gooditem){
      icon->setPixmap( LXDG::findIcon(item.icon, "preferences-system-windows-actions").pixmap(32,32) );
      iconPath = item.icon;
      text = item.name;
      if(!item.genericName.isEmpty() && item.name!=item.genericName){ text.append("<br><i> -- "+item.genericName+"</i>"); }
      name->setText(text);
      name->setToolTip(item.comment);
      setupActions(&item);
    }else{
      return;
    }
  }else if(type=="dir"){
    actButton->setVisible(false);
    if(itemPath.endsWith("/")){ itemPath.chop(1); }
    if(goback){
      icon->setPixmap( LXDG::findIcon("go-previous","").pixmap(64,64) );
      iconPath = "go-previous";
      text = tr("Go Back");
      name->setText( text );
    }else{
      icon->setPixmap( LXDG::findIcon("folder","").pixmap(64,64) );
      iconPath = "folder";
      name->setText( itemPath.section("/",-1));
      text = itemPath.section("/",-1);
    }
  }else if(type.startsWith("chcat::::")){
    //Category given
    actButton->setVisible(false);
    iconPath = LXDG::DesktopCatToIcon(type.section("::::",1,50));
    if(goback){ iconPath = "go-previous"; type = "chcat::::"; itemPath = "<B>("+itemPath+")</B>"; }
    icon->setPixmap( LXDG::findIcon(iconPath,"applications-other").pixmap(64,64) );
    name->setText(itemPath);
    text = itemPath;
    icon->setWhatsThis(type);
    linkPath = type;
  }else{
    actButton->setVisible(false);
    if(itemPath.endsWith("/")){ itemPath.chop(1); }
    if(QFileInfo(itemPath).isDir()){
      type = "dir";
      icon->setPixmap( LXDG::findIcon("folder","").pixmap(64,64) );
      iconPath = "folder";
    }else if(LUtils::imageExtensions().contains(itemPath.section("/",-1).section(".",-1).toLower()) ){
      icon->setPixmap( QIcon(itemPath).pixmap(64,64) );
    }else{
      if( LUtils::isValidBinary(itemPath) ){  icon->setPixmap( LXDG::findIcon(type, "application-x-executable").pixmap(64,64) ); }
      else{ icon->setPixmap( LXDG::findMimeIcon(itemPath.section("/",-1)).pixmap(64,64) ); }
    }
    name->setText( itemPath.section("/",-1) ); //this->fontMetrics().elidedText(itemPath.section("/",-1), Qt::ElideRight, TEXTCUTOFF) ); 
    text = itemPath.section("/",-1) ;
  }
  icon->setWhatsThis(itemPath);
  if(!goback){ this->setWhatsThis(name->text()); }
  isDirectory = (type=="dir"); //save this for later
  if(LDesktopUtils::isFavorite(itemPath)){
    linkPath = itemPath;
    isShortcut=true;
  }else if( inHome ){//|| itemPath.section("/",0,-2)==QDir::homePath()+"/Desktop" ){
    isShortcut = true;
  }else{
    isShortcut = false;
  }
  if(isShortcut && name->toolTip().isEmpty()){
    name->setToolTip(icon->whatsThis()); //also allow the user to see the full shortcut path
  }
  //Now setup the button appropriately
  setupContextMenu();
}

// - Application constructor
ItemWidget::ItemWidget(QWidget *parent, XDGDesktop *item) : QFrame(parent){
  createWidget();
  if(item==0){ gooditem = false; return; }
  isDirectory = false;
  if(LDesktopUtils::isFavorite(item->filePath)){
    linkPath = item->filePath;
    isShortcut=true;
  }else if( item->filePath.section("/",0,-2)==QDir::homePath()+"/Desktop" ){
    isShortcut = true;
  }else{
    isShortcut = false;
  }
  if(isShortcut){
    name->setToolTip(icon->whatsThis()); //also allow the user to see the full shortcut path
  }
  //Now fill it appropriately
  icon->setPixmap( LXDG::findIcon(item->icon,"preferences-system-windows-actions").pixmap(64,64) );
      text = item->name;
      if(!item->genericName.isEmpty() && item->name!=item->genericName){ text.append("<br><i> -- "+item->genericName+"</i>"); }
      name->setText(text);
      name->setToolTip(item->comment);
  this->setWhatsThis(item->name);
  icon->setWhatsThis(item->filePath);
  iconPath = item->icon;
  //Now setup the buttons appropriately
  setupContextMenu();
  setupActions(item);
}

ItemWidget::~ItemWidget(){
  icon->setPixmap(QPixmap()); //make sure the pixmap is cleared from memory too
  actButton->deleteLater();
  contextMenu->clear();
  contextMenu->deleteLater();
  if(actButton->menu()!=0){ 
    for(int i=0; i<actButton->menu()->actions().length(); i++){
      actButton->menu()->actions().at(i)->deleteLater();
    }
    actButton->menu()->deleteLater(); 
  }
  actButton->deleteLater();
  icon->deleteLater();
  name->deleteLater();
  menureset->deleteLater();
  linkPath.clear(); iconPath.clear(); text.clear();
}

void ItemWidget::triggerItem(){
  ItemClicked();
}

void ItemWidget::createWidget(){
  //Initialize the widgets
  gooditem = true;
  menuopen = false;
  menureset = new QTimer(this);
    menureset->setSingleShot(true);
    menureset->setInterval(1000); //1 second	
  this->setContentsMargins(0,0,0,0);
  contextMenu = new QMenu(this);
    connect(contextMenu, SIGNAL(aboutToShow()), this, SLOT(actionMenuOpen()) );
    connect(contextMenu, SIGNAL(aboutToHide()), this, SLOT(actionMenuClosed()) );
  actButton = new QToolButton(this);
    actButton->setPopupMode(QToolButton::InstantPopup);
    actButton->setArrowType(Qt::DownArrow);
  icon = new QLabel(this);
  name = new QLabel(this);
    name->setWordWrap(true);
    name->setTextFormat(Qt::RichText);
    name->setTextInteractionFlags(Qt::NoTextInteraction);
  //Add them to the layout
  this->setLayout(new QHBoxLayout(this));
    this->layout()->setContentsMargins(1,1,1,1);
    this->layout()->addWidget(icon);
    this->layout()->addWidget(actButton);
    this->layout()->addWidget(name);
  //Set a custom object name so this can be tied into the Lumina Theme stylesheets
  this->setObjectName("LuminaItemWidget");
}

void ItemWidget::setupContextMenu(){
  //Now refresh the context menu
  contextMenu->clear();
  if(!QFile::exists(QDir::homePath()+"/Desktop/"+icon->whatsThis().section("/",-1)) ){
    //Does not have a desktop link
    contextMenu->addAction( LXDG::findIcon("preferences-desktop-icons",""), tr("Pin to Desktop"), this, SLOT(PinToDesktop()) );
  }
  //Favorite Item
  if( LDesktopUtils::isFavorite(icon->whatsThis()) ){ //Favorite Item - can always remove this
    contextMenu->addAction( LXDG::findIcon("edit-delete",""), tr("Remove from Favorites"), this, SLOT(RemoveFavorite()) );
  }else{
    //This file does not have a shortcut yet -- allow the user to add it
    contextMenu->addAction( LXDG::findIcon("bookmark-toolbar",""), tr("Add to Favorites"), this, SLOT(AddFavorite()) );
  }
  //QuickLaunch Item
  if(LSession::handle()->sessionSettings()->value("QuicklaunchApps",QStringList()).toStringList().contains(icon->whatsThis()) ){ //Favorite Item - can always remove this
    contextMenu->addAction( LXDG::findIcon("edit-delete",""), tr("Remove from Quicklaunch"), this, SLOT(RemoveQL()) );
  }else{
    //This file does not have a shortcut yet -- allow the user to add it
    contextMenu->addAction( LXDG::findIcon("quickopen",""), tr("Add to Quicklaunch"), this, SLOT(AddQL()) );
  }
}

void ItemWidget::setupActions(XDGDesktop *app){
  if(app==0 || app->actions.isEmpty()){ actButton->setVisible(false); return; }
  //Actions Available - go ahead and list them all
  actButton->setMenu( new QMenu(this) );
  for(int i=0; i<app->actions.length(); i++){
    QAction *act = new QAction(LXDG::findIcon(app->actions[i].icon, app->icon), app->actions[i].name, this);
	act->setToolTip(app->actions[i].ID);
        act->setWhatsThis(app->actions[i].ID);
        actButton->menu()->addAction(act);	
  }
  connect(actButton->menu(), SIGNAL(triggered(QAction*)), this, SLOT(actionClicked(QAction*)) );
  connect(actButton->menu(), SIGNAL(aboutToShow()), this, SLOT(actionMenuOpen()) );
  connect(actButton->menu(), SIGNAL(aboutToHide()), this, SLOT(actionMenuClosed()) );
  connect(menureset, SIGNAL(timeout()), this, SLOT(resetmenuflag()) );
}

void ItemWidget::updateItems(){
  //update the text/icon to match sizes
  int H = 2.3*name->fontMetrics().height(); //make sure the height is large enough for two lines
  icon->setFixedSize(QSize(H-4, H-4));
  actButton->setFixedSize( QSize( (H-4)/2, H-4) );
  QStringList newname = text.split("<br>");
  for(int i=0; i<newname.length(); i++){ newname[i] = name->fontMetrics().elidedText(newname[i], Qt::ElideRight, name->width()); }
  name->setText( newname.join("<br>") );
  //Now reload the icon if necessary
  if(icon->pixmap()!=0){
    if(icon->pixmap()->size().height() < (H-4) ){
      if(iconPath.isEmpty()){
        //Use item path (thumbnail or mimetype)
        if(LUtils::imageExtensions().contains(icon->whatsThis().section("/",-1).section(".",-1).toLower()) ){
          icon->setPixmap( QIcon(icon->whatsThis()).pixmap(H-4,H-4).scaledToHeight(H-4,Qt::SmoothTransformation) );
        }else{
          icon->setPixmap( LXDG::findMimeIcon(icon->whatsThis().section("/",-1)).pixmap(H-4,H-4).scaledToHeight(H-4,Qt::SmoothTransformation) );
        }
      }else{
        icon->setPixmap( LXDG::findIcon(iconPath,"preferences-system-windows-actions").pixmap(H-4,H-4).scaledToHeight(H-4,Qt::SmoothTransformation) );
      }
    }else if(icon->pixmap()->size().height() > (H-4) ){
      icon->setPixmap( icon->pixmap()->scaled(H-4, H-4, Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );
    }
  }
}

void ItemWidget::PinToDesktop(){
  qDebug() << "Create Link on Desktop:" << icon->whatsThis();
  bool ok = QFile::link(icon->whatsThis(), QDir::homePath()+"/Desktop/"+icon->whatsThis().section("/",-1));
  qDebug() << " - " << (ok ? "Success": "Failure");
}

void ItemWidget::RemoveFavorite(){
  LDesktopUtils::removeFavorite(icon->whatsThis());
  linkPath.clear();
  emit RemovedShortcut();
}

void ItemWidget::AddFavorite(){
  if( LDesktopUtils::addFavorite(icon->whatsThis()) ){
    linkPath = icon->whatsThis();
    emit NewShortcut();	
  }
  
}
void ItemWidget::RemoveQL(){
  qDebug() << "Remove QuickLaunch Button:" << icon->whatsThis();
  emit toggleQuickLaunch(icon->whatsThis(), false);
}

void ItemWidget::AddQL(){
  qDebug() << "Add QuickLaunch Button:" << icon->whatsThis();
  emit toggleQuickLaunch(icon->whatsThis(), true);	
}


void ItemWidget::ItemClicked(){
  if(!linkPath.isEmpty()){ emit RunItem(linkPath); }
  else{ emit RunItem(icon->whatsThis()); }
}

void ItemWidget::actionClicked(QAction *act){
  actButton->menu()->hide();
  QString cmd = "lumina-open -action \""+act->whatsThis()+"\" \"%1\"";
  if(!linkPath.isEmpty()){ cmd = cmd.arg(linkPath); }
  else{ cmd = cmd.arg(icon->whatsThis()); }
  emit RunItem(cmd);
}
