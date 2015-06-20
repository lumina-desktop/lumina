//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "UserWidget.h"
#include "ui_UserWidget.h"
#include "../../LSession.h"
#include "../../AppMenu.h"

UserWidget::UserWidget(QWidget* parent) : QTabWidget(parent), ui(new Ui::UserWidget){
  ui->setupUi(this);
  if(parent!=0){ parent->setMouseTracking(true); }
  this->setMouseTracking(true);
  sysapps = LSession::handle()->applicationMenu()->currentAppHash(); //get the raw info
  //Setup the Icons
    // - favorites tab
    this->setTabIcon(0, rotateIcon(LXDG::findIcon("folder-favorites","")) );
    this->setTabText(0,"");
    // - apps tab
    this->setTabIcon(1, rotateIcon(LXDG::findIcon("system-run","")) );
    this->setTabText(1,"");
    // - home tab
    this->setTabIcon(2, rotateIcon(LXDG::findIcon("user-home","")) );
    this->setTabText(2,"");
    // - config tab
    this->setTabIcon(3, rotateIcon(LXDG::findIcon("preferences-system","")) );
    this->setTabText(3,"");
  ui->tool_fav_apps->setIcon( LXDG::findIcon("system-run","") );
  ui->tool_fav_dirs->setIcon( LXDG::findIcon("folder","") );
  ui->tool_fav_files->setIcon( LXDG::findIcon("document-multiple","") );
  ui->tool_desktopsettings->setIcon( LXDG::findIcon("preferences-desktop","") );
  ui->tool_config_screensaver->setIcon( LXDG::findIcon("preferences-desktop-screensaver","") );	
  ui->tool_config_screensettings->setIcon( LXDG::findIcon("preferences-other","") );
  ui->tool_home_gohome->setIcon( LXDG::findIcon("go-home","") );
  ui->tool_home_browse->setIcon( LXDG::findIcon("document-open","") );
  ui->tool_home_search->setIcon( LXDG::findIcon("system-search","") );
  ui->tool_config_about->setIcon( LXDG::findIcon("lumina","") );
  //Connect the signals/slots
  connect(ui->tool_desktopsettings, SIGNAL(clicked()), this, SLOT(openDeskSettings()) );
  connect(ui->tool_config_screensaver, SIGNAL(clicked()), this, SLOT(openScreenSaverConfig()) );
  connect(ui->tool_config_screensettings, SIGNAL(clicked()), this, SLOT(openScreenConfig()) );
  connect(ui->tool_fav_apps, SIGNAL(clicked()), this, SLOT(FavChanged()) );
  connect(ui->tool_fav_files, SIGNAL(clicked()), this, SLOT(FavChanged()) );
  connect(ui->tool_fav_dirs, SIGNAL(clicked()), this, SLOT(FavChanged()) );
  connect(ui->combo_app_cats, SIGNAL(currentIndexChanged(int)), this, SLOT(updateApps()) );
  connect(ui->tool_home_gohome, SIGNAL(clicked()), this, SLOT(slotGoHome()) );
  connect(ui->tool_home_browse, SIGNAL(clicked()), this, SLOT(slotOpenDir()) );
  connect(ui->tool_home_search, SIGNAL(clicked()), this, SLOT(slotOpenSearch()) );
  connect(ui->tool_config_about, SIGNAL(clicked()), this, SLOT(openLuminaInfo()) );
  
  //Setup the special buttons
  QString APPSTORE = LOS::AppStoreShortcut();
  if(QFile::exists(APPSTORE) && !APPSTORE.isEmpty()){
    //Now load the info
    bool ok = false;
    XDGDesktop store = LXDG::loadDesktopFile(APPSTORE, ok);
    if(ok){
      ui->tool_app_store->setIcon( LXDG::findIcon(store.icon, "") );
      ui->tool_app_store->setText( store.name );
      connect(ui->tool_app_store, SIGNAL(clicked()), this, SLOT(openStore()) );
    }
    ui->tool_app_store->setVisible(ok); //availability
  }else{
    ui->tool_app_store->setVisible(false); //not available
  }
  QString CONTROLPANEL = LOS::ControlPanelShortcut();
  if(QFile::exists(CONTROLPANEL) && !CONTROLPANEL.isEmpty()){
    //Now load the info
    bool ok = false;
    XDGDesktop cpan = LXDG::loadDesktopFile(CONTROLPANEL, ok);
    if(ok){
      ui->tool_controlpanel->setIcon( LXDG::findIcon(cpan.icon, "") );
      connect(ui->tool_controlpanel, SIGNAL(clicked()), this, SLOT(openControlPanel()) );
    }
    ui->tool_controlpanel->setVisible(ok); //availability
  }else{
    ui->tool_controlpanel->setVisible(false); //not available
  }
  QString QTCONFIG = LOS::QtConfigShortcut();
  if(QFile::exists(QTCONFIG) && !QTCONFIG.isEmpty()){
    ui->tool_qtconfig->setVisible(true);
    ui->tool_qtconfig->setIcon( LXDG::findIcon("preferences-desktop-theme","") );
    connect(ui->tool_qtconfig, SIGNAL(clicked()), this, SLOT(openQtConfig()) );
  }else{
    ui->tool_qtconfig->setVisible(false);
  }
  lastUpdate = QDateTime(); //make sure it refreshes 

  connect(LSession::handle()->applicationMenu(), SIGNAL(AppMenuUpdated()), this, SLOT(UpdateMenu()) );
  connect(QApplication::instance(), SIGNAL(DesktopFilesChanged()), this, SLOT(updateFavItems()) );
  QTimer::singleShot(10,this, SLOT(UpdateMenu())); //make sure to load this once after initialization
}

UserWidget::~UserWidget(){
}

//===========
//     PRIVATE
//===========
void UserWidget::ClearScrollArea(QScrollArea *area){
  QWidget *wgt = area->takeWidget();
  delete wgt; //delete the widget and all children
  area->setWidget( new QWidget() ); //create a new widget in the scroll area
  area->widget()->setContentsMargins(0,0,0,0);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(2);
    layout->setContentsMargins(3,1,3,1);
    layout->setDirection(QBoxLayout::TopToBottom);
    area->widget()->setLayout(layout);
}

QIcon UserWidget::rotateIcon(QIcon ico){
  //Rotate the given icon to appear vertical in the tab widget
  QPixmap pix = ico.pixmap(32,32);
  QTransform tran;
    tran.rotate(+90); //For tabs on the left/West
  pix = pix.transformed(tran);
  ico = QIcon(pix);
  return ico;
}

//============
//  PRIVATE SLOTS
//============
void UserWidget::UpdateMenu(){
    this->setCurrentWidget(ui->tab_fav);
    ui->tool_fav_apps->setChecked(true);
    ui->tool_fav_dirs->setChecked(false);
    ui->tool_fav_files->setChecked(false);
    cfav = 0; //favorite apps
    updateFavItems();
    ui->label_home_dir->setWhatsThis(QDir::homePath());
    updateHome();
  if(lastUpdate < LSession::handle()->applicationMenu()->lastHashUpdate || lastUpdate.isNull()){
    updateAppCategories();
    updateApps();
  }
  lastUpdate = QDateTime::currentDateTime();
}

void UserWidget::LaunchItem(QString path, bool fix){
  if(!path.isEmpty()){
    qDebug() << "Launch Application:" << path;
    if(fix){ LSession::LaunchApplication("lumina-open \""+path+"\""); }
    else{ LSession::LaunchApplication(path); }
    emit CloseMenu(); //so the menu container will close
  }
}

void UserWidget::FavChanged(){
  //uncheck the current item for a moment
  int oldfav = cfav;
  if(cfav==0){ ui->tool_fav_apps->setChecked(false); }
  else if(cfav==1){ ui->tool_fav_dirs->setChecked(false); }
  if(cfav==2){ ui->tool_fav_files->setChecked(false); }
  //Now check what other item is now the only one checked
  if(ui->tool_fav_apps->isChecked() && !ui->tool_fav_dirs->isChecked() && !ui->tool_fav_files->isChecked() ){
    cfav = 0;
  }else if(!ui->tool_fav_apps->isChecked() && ui->tool_fav_dirs->isChecked() && !ui->tool_fav_files->isChecked() ){
    cfav = 1;
  }else if(!ui->tool_fav_apps->isChecked() && !ui->tool_fav_dirs->isChecked() && ui->tool_fav_files->isChecked() ){
    cfav = 2;
  }else{
    //Re-check the old item (something invalid)
    ui->tool_fav_apps->setChecked(cfav==0);
    ui->tool_fav_dirs->setChecked(cfav==1);
    ui->tool_fav_files->setChecked(cfav==2);
  }
  updateFavItems(oldfav!=cfav);
}

void UserWidget::updateFavItems(bool newfilter){
  //qDebug() << "Updating User Favorite Items";
  QStringList newfavs = LUtils::listFavorites();
  //qDebug() << "Favorites:" << newfavs;
  if(lastHomeUpdate.isNull() || (QFileInfo(QDir::homePath()+"/Desktop").lastModified() > lastHomeUpdate) || newfavs!=favs ){
    favs = newfavs;
   
    homefiles = LSession::handle()->DesktopFiles();
    lastHomeUpdate = QDateTime::currentDateTime();
  }else if(!newfilter){ return; } //nothing new to change - stop now
  //qDebug() << " - Passed Smoke Test...";
  QStringList favitems;
  //Remember for format for favorites: <name>::::[app/dir/<mimetype>]::::<full path>
  if(ui->tool_fav_apps->isChecked()){ 
    favitems = favs.filter("::::app::::");
    for(int i=0; i<homefiles.length(); i++){
      if(homefiles[i].fileName().endsWith(".desktop")){
        favitems << homefiles[i].fileName()+"::::app-home::::"+homefiles[i].absoluteFilePath();
      }
    }
  }else if(ui->tool_fav_dirs->isChecked()){ 
    favitems = favs.filter("::::dir::::");
    for(int i=0; i<homefiles.length(); i++){
      if(homefiles[i].isDir()){
        favitems << homefiles[i].fileName()+"::::dir-home::::"+homefiles[i].absoluteFilePath();
      }
    }
  }else{ 
    //Files
    for(int i=0; i<favs.length(); i++){
      QString type = favs[i].section("::::",1,1);
      if(type != "app" && type !="dir"){
        favitems << favs[i];
      }
    }
    for(int i=0; i<homefiles.length(); i++){
      if(!homefiles[i].isDir() && !homefiles[i].fileName().endsWith(".desktop") ){
        favitems << homefiles[i].fileName()+"::::"+LXDG::findAppMimeForFile(homefiles[i].fileName())+"-home::::"+homefiles[i].absoluteFilePath();
      }
    }
  }
  ClearScrollArea(ui->scroll_fav);
  //qDebug() << " - Sorting Items";
  favitems.sort(); //sort them alphabetically
  //qDebug() << " - Creating Items:" << favitems;
  for(int i=0; i<favitems.length(); i++){
    UserItemWidget *it = new UserItemWidget(ui->scroll_fav->widget(), favitems[i].section("::::",2,50), favitems[i].section("::::",1,1) );
    ui->scroll_fav->widget()->layout()->addWidget(it);
    connect(it, SIGNAL(RunItem(QString)), this, SLOT(LaunchItem(QString)) );
    connect(it, SIGNAL(NewShortcut()), this, SLOT(updateFavItems()) );
    connect(it, SIGNAL(RemovedShortcut()), this, SLOT(updateFavItems()) );
  }
  static_cast<QBoxLayout*>(ui->scroll_fav->widget()->layout())->addStretch();
  //qDebug() << " - Done";
}

//Apps Tab
void UserWidget::updateAppCategories(){
  ui->combo_app_cats->clear();
  QStringList cats = sysapps->keys();
  cats.sort();
  for(int i=0; i<cats.length(); i++){
    QString name, icon;
    if(cats[i] == "All"){ name = tr("All"); icon = "application-x-executable"; }
    else if(cats[i] == "Multimedia"){ name = tr("Multimedia"); icon = "applications-multimedia"; }
    else if(cats[i] == "Development"){ name = tr("Development"); icon = "applications-development"; }
    else if(cats[i] == "Education"){ name = tr("Education"); icon = "applications-education"; }
    else if(cats[i] == "Game"){ name = tr("Games"); icon = "applications-games"; }
    else if(cats[i] == "Graphics"){ name = tr("Graphics"); icon = "applications-graphics"; }
    else if(cats[i] == "Network"){ name = tr("Network"); icon = "applications-internet"; }
    else if(cats[i] == "Office"){ name = tr("Office"); icon = "applications-office"; }
    else if(cats[i] == "Science"){ name = tr("Science"); icon = "applications-science"; }
    else if(cats[i] == "Settings"){ name = tr("Settings"); icon = "preferences-system"; }
    else if(cats[i] == "System"){ name = tr("System"); icon = "applications-system"; }
    else if(cats[i] == "Utility"){ name = tr("Utilities"); icon = "applications-utilities"; }
    else if(cats[i] == "Wine"){ name = tr("Wine"); icon = "wine"; }
    else{ name = tr("Unsorted"); icon = "applications-other"; }
    ui->combo_app_cats->addItem( LXDG::findIcon(icon,""), name, cats[i] );
  }
}

void UserWidget::updateApps(){
  if(ui->combo_app_cats->currentIndex() < 0){ return; } //no cat
  QString cat = ui->combo_app_cats->itemData( ui->combo_app_cats->currentIndex() ).toString();
  QList<XDGDesktop> items = sysapps->value(cat);
  ClearScrollArea(ui->scroll_apps);
  for(int i=0; i<items.length(); i++){
    UserItemWidget *it = new UserItemWidget(ui->scroll_apps->widget(), items[i]);
    ui->scroll_apps->widget()->layout()->addWidget(it);
    connect(it, SIGNAL(RunItem(QString)), this, SLOT(LaunchItem(QString)) );
    connect(it, SIGNAL(NewShortcut()), this, SLOT(updateFavItems()) );
    connect(it, SIGNAL(RemovedShortcut()), this, SLOT(updateFavItems()) );
  }
  static_cast<QBoxLayout*>(ui->scroll_apps->widget()->layout())->addStretch();
}

//Home Tab
void UserWidget::updateHome(){
  ClearScrollArea(ui->scroll_home);
  QDir homedir(ui->label_home_dir->whatsThis());
  QStringList items;
  if(QDir::homePath() == homedir.absolutePath()){
    ui->label_home_dir->setText(tr("Home"));
    ui->tool_home_gohome->setVisible(false);
  }else{
    ui->tool_home_gohome->setVisible(true);
    ui->label_home_dir->setText( this->fontMetrics().elidedText(homedir.dirName(), Qt::ElideRight, ui->label_home_dir->width()));
    //Now make sure to put a "go back" button at the top of the list
    QString dir = ui->label_home_dir->whatsThis();
    if(dir.endsWith("/")){ dir.chop(1); }
    dir.chop( dir.section("/",-1).length() );
    items << dir;
  }
  ui->label_home_dir->setToolTip(ui->label_home_dir->whatsThis());
  items << homedir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name); 
  QString type = "dir";
  if(homedir.absolutePath() == QDir::homePath()+"/Desktop"){ type.append("-home"); }//internal code
  for(int i=0; i<items.length(); i++){
    //qDebug() << "New Home subdir:" << homedir.absoluteFilePath(items[i]);
    UserItemWidget *it;
    if(items[i].startsWith("/")){ it = new UserItemWidget(ui->scroll_home->widget(), items[i], type, true); }
    else{ it = new UserItemWidget(ui->scroll_home->widget(), homedir.absoluteFilePath(items[i]), type, false); }
    ui->scroll_home->widget()->layout()->addWidget(it);
    connect(it, SIGNAL(RunItem(QString)), this, SLOT(slotGoToDir(QString)) );
    connect(it, SIGNAL(NewShortcut()), this, SLOT(updateFavItems()) );
    connect(it, SIGNAL(RemovedShortcut()), this, SLOT(updateFavItems()) );
  }
  static_cast<QBoxLayout*>(ui->scroll_home->widget()->layout())->addStretch();
}

void UserWidget::slotGoToDir(QString dir){
  ui->label_home_dir->setWhatsThis(dir);
  updateHome();
}

void UserWidget::slotGoHome(){
  slotGoToDir(QDir::homePath());
}
	
void UserWidget::slotOpenDir(){
  LaunchItem(ui->label_home_dir->whatsThis());
}

void UserWidget::slotOpenSearch(){
  LaunchItem("lumina-search -dir \""+ui->label_home_dir->whatsThis()+"\"", false); //use command as-is
}

void UserWidget::mouseMoveEvent( QMouseEvent *event){
  QTabBar *wid = tabBar();
  if(wid==0){ return; } //invalid widget found
  QPoint relpos = wid->mapFromGlobal( this->mapToGlobal(event->pos()) );
  //qDebug() << "Mouse Move Event: " << event->pos().x() << event->pos().y() << relpos.x() << relpos.y() << wid->width() << wid->height();
  if(wid && wid->tabAt(relpos) != -1){
    qDebug() << " - Mouse over tab";
    this->setCurrentIndex( wid->tabAt(relpos) );
  }
}
