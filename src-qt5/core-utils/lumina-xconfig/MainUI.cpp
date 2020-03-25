//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainUI.h"
#include "ui_MainUI.h"

#include <LuminaXDG.h>
#include <LUtils.h>

#include <QTimer>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI){
  ui->setupUi(this);
  loadIcons();
  scaleFactor = 1/15.0; //simple default value
  ui->combo_rotation->clear();
    ui->combo_rotation->addItem(tr("None"), 0);
    ui->combo_rotation->addItem(tr("Left"), -90);
    ui->combo_rotation->addItem(tr("Right"), 90);
    ui->combo_rotation->addItem(tr("Inverted"), 180);

  singleTileMenu = new QMenu(this);
    singleTileMenu->addAction(tr("Align Horizontally"))->setWhatsThis("X");
    singleTileMenu->addAction(tr("Align Vertically"))->setWhatsThis("Y");
    singleTileMenu->addAction(tr("Align Horizontal then Vertical"))->setWhatsThis("XY");
    singleTileMenu->addAction(tr("Align Vertical then Horizontal"))->setWhatsThis("YX");
  ui->mdiArea->setContextMenuPolicy(Qt::CustomContextMenu);

  profilesMenu = new QMenu(this);
    ui->tool_save->setMenu(profilesMenu);

  connect(ui->mdiArea, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu()) );

  connect(singleTileMenu, SIGNAL(triggered(QAction*)), this, SLOT(tileSingleScreen(QAction*)) );
  connect(ui->push_close, SIGNAL(clicked()), this, SLOT(close()) );
  connect(ui->push_rescan, SIGNAL(clicked()), this, SLOT(UpdateScreens()) );
  connect(ui->push_activate, SIGNAL(clicked()), this, SLOT(ActivateScreen()) );
  connect(ui->tool_deactivate, SIGNAL(clicked()), this, SLOT(DeactivateScreen()) );

  connect(ui->tool_save, SIGNAL(clicked()), this, SLOT(SaveSettings()) );
  connect(ui->tool_applyconfig, SIGNAL(clicked()), this, SLOT(ApplyChanges()) );
  connect(ui->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),this, SLOT(ScreenSelected()) );
  connect(ui->tool_tileX, SIGNAL(clicked()), this, SLOT(tileScreensX()) );
  connect(ui->tool_tileY, SIGNAL(clicked()), this, SLOT(tileScreensY()) );
  connect(ui->tool_tile, SIGNAL(clicked()), this, SLOT(tileScreens()) );
  connect(ui->combo_availscreens, SIGNAL(currentIndexChanged(int)), this, SLOT(updateNewScreenResolutions()) );

  connect(ui->tool_profile_load, SIGNAL(clicked()), this, SLOT(loadProfile()) );
  connect(ui->tool_profile_remove, SIGNAL(clicked()), this, SLOT(removeProfile()) );
  connect(profilesMenu, SIGNAL(triggered(QAction*)), this, SLOT(saveAsProfile(QAction*)) );

  updateProfiles();
  QTimer::singleShot(0, this, SLOT(UpdateScreens()) );
}

MainUI::~MainUI(){

}

void MainUI::loadIcons(){
  this->setWindowIcon( LXDG::findIcon("preferences-system-windows-actions","") );
  ui->tool_deactivate->setIcon( LXDG::findIcon("list-remove","") );

  ui->push_activate->setIcon( LXDG::findIcon("list-add","") );
  ui->push_rescan->setIcon( LXDG::findIcon("view-refresh","") );
  ui->push_close->setIcon( LXDG::findIcon("window-close","") );
  ui->tabWidget->setTabIcon(0, LXDG::findIcon("preferences-desktop-display","") );
  ui->tabWidget->setTabIcon(1, LXDG::findIcon("list-add","") );
  ui->tool_applyconfig->setIcon( LXDG::findIcon("dialog-ok-apply","") );
}

QString MainUI::currentSelection(){
  QMdiSubWindow *tmp = ui->mdiArea->activeSubWindow();
  if(tmp!=0){ return tmp->whatsThis(); }
  else{ return ""; }
}

ScreenInfo MainUI::currentScreenInfo(){
  QString item = currentSelection();
  if(item!=0){
    for(int i=0; i<SCREENS.length(); i++){
      if(SCREENS[i].ID==item){ return SCREENS[i]; }
    }
  }
  //Fallback when nothing found/selected
  return ScreenInfo();
}

void MainUI::AddScreenToWidget(ScreenInfo screen){
  //qDebug() << "Add Screen To Widget:" << screen.ID << screen.geom;
  QLabel *lab = new QLabel(this);
  lab->setAlignment(Qt::AlignCenter);
  QMdiSubWindow *it = ui->mdiArea->addSubWindow(lab, Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  it->setSystemMenu(0);
  it->setWindowTitle(screen.ID);
  lab->setText(QString::number(screen.geom.width())+"x"+QString::number(screen.geom.height()));

  it->setWhatsThis(screen.ID);
  QRect scaled( screen.geom.topLeft()*scaleFactor, screen.geom.size()*scaleFactor);
  //qDebug() << " - Scaled:" << scaled;
  it->show();
  it->setGeometry(scaled); //scale it down for the display
  it->setFixedSize(scaled.size());

}

void MainUI::SyncBackend(){
  QString item = currentSelection();
  if(item.isEmpty()){ return; } //nothing to do
  QString newres = ui->combo_resolution->currentData().toString().section("(",0,0).simplified();
  if(newres.isEmpty()){ return; } //nothing to do
  //qDebug() << "Apply Screen Changes" << item << "->" << newres;
  //Adjust the order of the two screens
  bool setprimary = ui->check_primary->isChecked();
  QList<QMdiSubWindow*> windows = ui->mdiArea->subWindowList();
  for(int i=0; i<SCREENS.length(); i++){
    if(SCREENS[i].ID == item){
      SCREENS[i].geom.setWidth(newres.section("x",0,0).toInt());
      SCREENS[i].geom.setHeight(newres.section("x",1,1).toInt());
      //qDebug() << " - New Geom:" << SCREENS[i].geom;
      SCREENS[i].rotation = ui->combo_rotation->currentData().toInt();
    }
    if(setprimary){ SCREENS[i].isprimary = (SCREENS[i].ID==item); }
    //Find the window associated with this screen
    for(int s=0; s<windows.length(); s++){
      if(windows[s]->whatsThis()==SCREENS[i].ID){
        //qDebug() << "Adjust geom of window:" << SCREENS[i].geom;
        SCREENS[i].geom.moveTopLeft( windows[s]->geometry().topLeft()/scaleFactor );
        //qDebug() << " - New Geom:" << SCREENS[i].geom;
        if(SCREENS[i].applyChange<1){ SCREENS[i].applyChange = (windows[s]->widget()->isEnabled() ? 0 : 1); } //disabled window is one that will be removed
      }
    }
  }
}

void MainUI::UpdateScreens(QString profile){
  //First probe the server for current screens
  if(profile.isEmpty()){ SCREENS = RRSettings::CurrentScreens(); }
  else{ SCREENS = RRSettings::PreviousSettings(profile); }

  //Determine the scale factor for putting these into the UI
  QRegion tot;
  for(int i=0; i<SCREENS.length(); i++){
    if(SCREENS[i].isactive){ tot = tot.united( SCREENS[i].geom ); }
  }
  int wid = tot.boundingRect().width();
  scaleFactor = (0.9*ui->mdiArea->width())/wid;
  //Now go through the screens and arrange them in order from left->right in the UI
  //bool found = true;
  //int xoffset = 0; //start at 0
  //int cnum = 0;
  QString csel = currentSelection();
  //Clear all the current widgets
  while(ui->mdiArea->currentSubWindow()!=0 ){
    QMdiSubWindow *tmp = ui->mdiArea->currentSubWindow();
    tmp->widget()->deleteLater();
    ui->mdiArea->removeSubWindow(tmp);
    tmp->deleteLater();
  }
  //Now add all the active screens to the display
  for(int i=0; i<SCREENS.length(); i++){
    if(SCREENS[i].isactive){ AddScreenToWidget(SCREENS[i]); }
  }

  //Now update the available/current screens in the UI
  ui->combo_availscreens->clear();
  for(int i=0; i<SCREENS.length(); i++){
    if(!SCREENS[i].isactive && SCREENS[i].isavailable){
      ui->combo_availscreens->addItem(SCREENS[i].ID);
    }
  }
  if(ui->combo_availscreens->count()<1){
    ui->group_avail->setVisible(false);
    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->setTabEnabled(1,false);
  }else{
    ui->group_avail->setVisible(true);
    ui->tabWidget->setTabEnabled(1,true);
  }
  ScreenSelected(); //update buttons
  updateNewScreenResolutions();
}

void MainUI::ScreenSelected(){
  QString item = currentSelection();
  //QListWidgetItem *item = ui->list_screens->currentItem();
  if(item.isEmpty()){
    //nothing selected
    ui->tool_deactivate->setEnabled(false);
    ui->tab_config->setEnabled(false);
  }else{
    //Item selected
    ui->tool_deactivate->setEnabled(true);
    ui->tab_config->setEnabled(true);
    //Update the info available on the config tab
    ScreenInfo cur = currentScreenInfo();
    ui->combo_resolution->clear();
    QString cres = QString::number(cur.geom.width())+"x"+QString::number(cur.geom.height());
    for(int i=0; i<cur.resList.length(); i++){
      QString res = cur.resList[i].section(" ",0,0, QString::SectionSkipEmpty);
      if(cur.resList[i].contains("+")){ ui->combo_resolution->addItem( QString(tr("%1 (Recommended)")).arg(res), res); }
      else{ui->combo_resolution->addItem(res, res); }
      if(cur.resList[i].contains(cres)){ ui->combo_resolution->setCurrentIndex(i); }
    }
    ui->check_primary->setChecked( cur.isprimary );
    int index = ui->combo_rotation->findData( cur.rotation );
    if(index<0){ index = 0; }
    ui->combo_rotation->setCurrentIndex(index);
  }
}

void MainUI::updateNewScreenResolutions(){
  QString id = ui->combo_availscreens->currentText();
  for(int i=0; i<SCREENS.length(); i++){
    if(SCREENS[i].ID==id){
      ui->combo_resolutions->clear();
      for(int r=0; r<SCREENS[i].resList.length(); r++){
        ui->combo_resolutions->addItem(SCREENS[i].resList[r].section(" ",0,0, QString::SectionSkipEmpty));
        if(SCREENS[i].resList[r].contains("+")){ ui->combo_resolutions->setCurrentIndex(r); } //preferred resolution
      }
      break;
    }
  }
}

void MainUI::tileScreensY(bool activeonly){
  //qDebug() << "Tile Windows in Y Dimension";
  QList<QMdiSubWindow*> wins = ui->mdiArea->subWindowList();
  QMdiSubWindow *active = ui->mdiArea->currentSubWindow();
  QRegion total;
  int ypos = 0;
  QMdiSubWindow *cur = 0;
  while(!wins.isEmpty()){
    cur=0;
    for(int i=0; i<wins.length(); i++){
      if(wins[i]==active && wins.length()>1){ continue; } //ensure active window is last
      if(cur==0){ cur = wins[i]; } //first one
      else if(wins[i]->pos().y() < cur->pos().y()){ cur = wins[i]; }
    }
    if(cur==0){
      //Note: This should **never** happen
      qDebug() << "No windows found below y=:" << ypos;
      //need to move the reference point
      QRect bounding = total.boundingRect();
      ypos+= (bounding.height()/2);
    }else{
      if(total.isNull()){
        //First window handled
        if(!activeonly || cur==active){ cur->move(cur->pos().x(), ypos); }
      }else{
        int newy = ypos;
        bool overlap = true;
        while(overlap){
          QRegion tmp(cur->pos().x(), newy, cur->width(), cur->height());
          QRegion diff = tmp.subtracted(total);
          overlap = (diff.boundingRect()!=tmp.boundingRect() || diff.rectCount()>1);
          //qDebug() << "Check Y Overlap:" << newy << overlap << tmp.boundingRect() << diff.boundingRect();
          if(overlap){
            QRect bound = diff.boundingRect();
            //qDebug() << " - got Y overlap:" << bound << rects;
            if( bound.isNull() || diff.rectCount() == 0 ){ newy+=cur->height(); }
            else{
             int orig = newy;
              newy = bound.top();
              for( const QRect &rect : diff ){
                //qDebug() << " - Check Rect:" << newy << rects[i] << cur->height();
                if(rect.height()==cur->height()){ continue; } //skip this one - just the same height
                if(rect.top()>newy || newy==bound.bottom()){ newy=rect.top(); }
              }
              if(orig==newy){ newy = bound.bottom()+1; } //make sure it always changes - no infinite loops!!
            }
          }
        }
        if(!activeonly || cur==active){ cur->move(cur->pos().x(), newy); }
      }
      total = total.united(cur->geometry());
      wins.removeAll(cur);
    }
  }
}

void MainUI::tileScreensX(bool activeonly){
  //qDebug() << "Tile Windows in X Dimension";
  QList<QMdiSubWindow*> wins = ui->mdiArea->subWindowList();
  QMdiSubWindow *active = ui->mdiArea->currentSubWindow();
  QRegion total;
  int xpos = 0;
  QMdiSubWindow *cur = 0;
  while(!wins.isEmpty()){
    cur=0;
    for(int i=0; i<wins.length(); i++){
      if(wins[i]==active && wins.length()>1){ continue; } //ensure active window is last
      if(cur==0){ cur = wins[i]; } //first one
      else if(wins[i]->pos().x() < cur->pos().x()){ cur = wins[i]; }
    }
    if(cur==0){
      //Note: This should **never** happen
      qDebug() << "No windows found left of x=:" << xpos;
      //need to move the reference point
      QRect bounding = total.boundingRect();
      xpos+= (bounding.width()/2);
    }else{
      if(total.isNull()){
        //First window handled
        if(!activeonly || cur==active){ cur->move(xpos, cur->pos().y()); }
      }else{
        int newx = xpos;
        bool overlap = true;
        while(overlap){
          QRegion tmp(newx, cur->pos().y(), cur->width(), cur->height());
          QRegion diff = tmp.subtracted(total);
          overlap = (diff.boundingRect()!=tmp.boundingRect() || diff.rectCount()>1);
          //qDebug() << "Check X Overlap:" << newx << overlap << total.rects() << tmp.boundingRect() << diff.boundingRect();
          if(overlap){
            QRect bound = diff.boundingRect();
            //qDebug() << " - got X overlap:" << bound << rects;
            if( bound.isNull() || diff.rectCount() == 0 ){ newx+=cur->width(); }
            else{
             int orig = newx;
              newx = bound.left();
              for( const QRect &rect : diff ) {
                //qDebug() << " - Check Rect:" << newx << rects[i] << cur->width();
                if(rect.width()==cur->width()){ continue; } //skip this one - just the same width
                if(rect.left()>newx || newx==bound.right()){ newx=rect.left(); }
              }
              if(orig==newx){ newx = bound.right()+1; } //make sure it always changes - no infinite loops!!
            }
          }
        }
        if(!activeonly || cur==active){ cur->move(newx, cur->pos().y()); }
      }
      total = total.united(cur->geometry());
      wins.removeAll(cur);
    }
  }
}

void MainUI::tileScreens(){
  tileScreensY();
  tileScreensX();
}

void MainUI::tileSingleScreen(QAction* act){
  if(act->whatsThis()=="X"){
    tileScreensX(true);
  }else if(act->whatsThis()=="Y"){
    tileScreensY(true);
  }else if(act->whatsThis()=="XY"){
    tileScreensX(true);
    tileScreensY(true);
  }else if(act->whatsThis()=="YX"){
    tileScreensY(true);
    tileScreensX(true);
  }
}

void MainUI::DeactivateScreen(){
  QMdiSubWindow *cur = ui->mdiArea->currentSubWindow();
  if(cur==0){ return; }
  cur->widget()->setEnabled( !cur->widget()->isEnabled() ); //toggle it between enabled/disabled
}

void MainUI::ActivateScreen(){
  //Assemble the command;
  QString ID = ui->combo_availscreens->currentText();
  QStringList res = ui->combo_resolutions->currentText().split("x");
  //Find the screen infor associated with this ID
  for(int i=0; i<SCREENS.length(); i++){
    if(SCREENS[i].ID==ID){
      SCREENS[i].isactive = true;
      SCREENS[i].geom.setSize(  QSize(res[0].toInt(), res[1].toInt()) );
      SCREENS[i].applyChange = 2; //need to activate this monitor
      AddScreenToWidget(SCREENS[i]);
      break;
    }
  }
  tileScreensX(true);
  //Now remove that option from the "new" list
  ui->combo_availscreens->removeItem(ui->combo_availscreens->currentIndex());
  if(ui->combo_availscreens->count()<1){
    ui->group_avail->setVisible(false);
    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->setTabEnabled(1,false);
  }else{
    ui->group_avail->setVisible(true);
    ui->tabWidget->setTabEnabled(1,true);
  }
}

void MainUI::ApplyChanges(){
  SyncBackend();
  //Now run the command
  RRSettings::Apply(SCREENS);
  //And update the UI and WM in a moment
  QTimer::singleShot(500, this, SLOT(UpdateScreens()) );
  QTimer::singleShot(1000, this, SLOT(RestartFluxbox()) );
}

void MainUI::SaveSettings(bool quiet){
  SyncBackend();
  bool ok = RRSettings::SaveScreens(SCREENS);
  if(quiet){ return; } //do not show the popup info boxes.
  if(ok){
    QMessageBox::information(this, tr("Settings Saved"), tr("Screen configuration saved as the default for future use"));
  }else{
    QMessageBox::warning(this, tr("Settings Error"), tr("Screen configuration could not be saved. Please check file permissions and try again."));
  }
}

void MainUI::RestartFluxbox(){
  QProcess::startDetached("pkill fluxbox");
}

void MainUI::removeProfile(){
  QString cur = ui->combo_profiles->currentText();
  RRSettings::removeProfile(cur);
  updateProfiles();
}

void MainUI::updateProfiles(){
  QStringList profiles = RRSettings::savedProfiles();
  ui->combo_profiles->clear();
  profiles.sort();
  ui->combo_profiles->addItems(profiles);
  //Update the profiles menu as needed
  profilesMenu->clear();
  for(int i=0; i<profiles.length(); i++){
    profilesMenu->addAction(profiles[i])->setWhatsThis(profiles[i]);
  }
  if(!profiles.isEmpty()){ profilesMenu->addSeparator(); }
  profilesMenu->addAction(tr("New Profile") );

  //Now update the tab as needed
  ui->tabWidget->setTabEnabled(2,!profiles.isEmpty());
  if(ui->tabWidget->currentIndex()==2){ ui->tabWidget->setCurrentIndex(0); }
}

void MainUI::loadProfile(){
  QString cur = ui->combo_profiles->currentText();
  UpdateScreens(cur);
}

void MainUI::saveAsProfile(QAction *act){
  QString profile = act->whatsThis();
  if(profile.isEmpty()){
    //Need to prompt for a profile name
    QStringList known = RRSettings::savedProfiles();
    while(known.contains(profile) || profile.isEmpty()){
      bool ok = false;
      profile = QInputDialog::getText(this, tr("Create Screen Profile"), profile.isEmpty() ? tr("Profile Name") : tr("Profile exists - different name:"), QLineEdit::Normal, profile, &ok);
      if(!ok || profile.isEmpty()){ return; } //cancelled
    }
  }
  bool ok = RRSettings::SaveScreens(SCREENS, profile);
  updateProfiles();
  if(ok){
    QMessageBox::information(this, tr("Profile Created"), tr("Current screen configuration saved as profile:")+"\n\n"+profile);
  }
}
