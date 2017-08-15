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

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI){
  ui->setupUi(this);
  loadIcons();
  scaleFactor = 1/20.0;
  //Fill the location list with the valid entries
  ui->combo_location->clear();
    ui->combo_location->addItem(tr("Right Of"), "--right-of");
    ui->combo_location->addItem(tr("Left Of"), "--left-of");
  ui->combo_rotation->clear();
    ui->combo_rotation->addItem(tr("None"), 0);
    ui->combo_rotation->addItem(tr("Left"), -90);
    ui->combo_rotation->addItem(tr("Right"), 90);
    ui->combo_rotation->addItem(tr("Inverted"), 180);

  connect(ui->push_close, SIGNAL(clicked()), this, SLOT(close()) );
  connect(ui->push_rescan, SIGNAL(clicked()), this, SLOT(UpdateScreens()) );
  connect(ui->push_activate, SIGNAL(clicked()), this, SLOT(ActivateScreen()) );
  connect(ui->tool_deactivate, SIGNAL(clicked()), this, SLOT(DeactivateScreen()) );
  //connect(ui->tool_moveleft, SIGNAL(clicked()), this, SLOT(MoveScreenLeft()) );
  //connect(ui->tool_moveright, SIGNAL(clicked()), this, SLOT(MoveScreenRight()) );
  connect(ui->tool_save, SIGNAL(clicked()), this, SLOT(SaveSettings()) );
  connect(ui->tool_applyconfig, SIGNAL(clicked()), this, SLOT(ApplyChanges()) );
  connect(ui->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),this, SLOT(ScreenSelected()) );
  connect(ui->tool_tile, SIGNAL(clicked()), ui->mdiArea, SLOT(tileSubWindows()) );
  QTimer::singleShot(0, this, SLOT(UpdateScreens()) );
}

MainUI::~MainUI(){

}

void MainUI::loadIcons(){
  this->setWindowIcon( LXDG::findIcon("preferences-system-windows-actions","") );
  ui->tool_deactivate->setIcon( LXDG::findIcon("list-remove","") );
  //ui->tool_moveleft->setIcon( LXDG::findIcon("arrow-left","") );
  //ui->tool_moveright->setIcon( LXDG::findIcon("arrow-right","") );
  ui->push_activate->setIcon( LXDG::findIcon("list-add","") );
  ui->push_rescan->setIcon( LXDG::findIcon("view-refresh","") );
  ui->push_close->setIcon( LXDG::findIcon("window-close","") );
  ui->tabWidget->setTabIcon(0, LXDG::findIcon("preferences-desktop-display","") );
  ui->tabWidget->setTabIcon(1, LXDG::findIcon("list-add","") );
  ui->tool_applyconfig->setIcon( LXDG::findIcon("dialog-ok-apply","") );
}

QStringList MainUI::currentOpts(){
  //Read all the settings and create the xrandr options to maintain these settings
  QStringList opts;
  for(int i=0; i<SCREENS.length(); i++){
    if(SCREENS[i].order <0){ continue; } //skip this screen - non-active
    opts << "--output" << SCREENS[i].ID << "--mode" << QString::number(SCREENS[i].geom.width())+"x"+QString::number(SCREENS[i].geom.height());
    if(SCREENS[i].isprimary){ opts << "--primary"; }
    if(SCREENS[i].order > 0){
      //Get the ID of the previous screen
      QString id;
      for(int j=0; j<SCREENS.length(); j++){
        if(SCREENS[j].order == SCREENS[i].order-1){ id = SCREENS[j].ID; break;}
      }
      if(!id.isEmpty()){ opts << "--right-of" << id; }
    }
  }
  return opts;
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
  it->setWindowTitle(screen.ID);
  lab->setText(QString::number(screen.geom.width())+"x"+QString::number(screen.geom.height()));

  /*QListWidgetItem *it = new QListWidgetItem();
  it->setTextAlignment(Qt::AlignCenter);
  it->setText( screen.ID+"\n\n ("+QString::number(screen.geom.x())+", "+QString::number(screen.geom.y())+")\n ("+QString::number(screen.geom.width())+"x"+QString::number(screen.geom.height())+")  " );*/
  it->setWhatsThis(screen.ID);
  //ui->list_screens->addItem(it);
  QRect scaled( screen.geom.topLeft()*scaleFactor, screen.geom.size()*scaleFactor);
  qDebug() << " - Scaled:" << scaled;
  it->show();
  //QApplication::processEvents();
  it->setGeometry(scaled); //scale it down for the display
  it->setFixedSize(scaled.size());

}

void MainUI::SyncBackend(){
  QString item = currentSelection();
  if(item.isEmpty()){ return; } //nothing to do
  QString newres = ui->combo_resolution->currentData().toString();
  if(newres.isEmpty()){ return; } //nothing to do
  //qDebug() << "Apply Screen Changes" << it->whatsThis() << "->" << newres;
  //Adjust the order of the two screens
  bool setprimary = ui->check_primary->isChecked();
  QList<QMdiSubWindow*> windows = ui->mdiArea->subWindowList();
  for(int i=0; i<SCREENS.length(); i++){
    if(SCREENS[i].ID == item){
      SCREENS[i].geom.setWidth(newres.section("x",0,0).toInt());
      SCREENS[i].geom.setHeight(newres.section("x",1,1).toInt());
      SCREENS[i].rotation = ui->combo_rotation->currentData().toInt();
    }
    if(setprimary){ SCREENS[i].isprimary = SCREENS[i].ID==item; }
    //Find the window associated with this screen
    for(int s=0; s<windows.length(); s++){
      if(windows[s]->whatsThis()==SCREENS[i].ID){
        SCREENS[i].geom.setTopLeft( windows[s]->geometry().topLeft()/scaleFactor );
      }
    }
  }
}

void MainUI::UpdateScreens(){
  //First probe the server for current screens
  SCREENS = RRSettings::CurrentScreens();
  //Now go through the screens and arrange them in order from left->right in the UI
  bool found = true;
  int xoffset = 0; //start at 0
  int cnum = 0;
  QString csel = currentSelection();
  //Clear all the current widgets
  while(ui->mdiArea->currentSubWindow()!=0 ){
    QMdiSubWindow *tmp = ui->mdiArea->currentSubWindow();
    tmp->widget()->deleteLater();
    ui->mdiArea->removeSubWindow(tmp);
    tmp->deleteLater();
  }

  while(found){
    found = false; //make sure to break out if a screen is not found
    for(int i=0; i<SCREENS.length(); i++){
      if(SCREENS[i].order != -1){qDebug() << "Skip Screen:" << i << SCREENS[i].order; } //already evaluated - skip it
      else if(SCREENS[i].geom.x()==xoffset){
	found = true; //make sure to look for the next one
	xoffset = xoffset+SCREENS[i].geom.width(); //next number to look for
	SCREENS[i].order = cnum; //assign the current order to it
	cnum++; //get ready for the next one
         AddScreenToWidget(SCREENS[i]);
      }else if(SCREENS[i].geom.x() < xoffset || SCREENS[i].geom.x() > xoffset){
        //Screen not aligned with previous screen edge
        qDebug() << "Found mis-aligned screen:" << i << SCREENS[i].ID;
        found = true; //make sure to look for the next one
        xoffset = xoffset+SCREENS[i].geom.width(); //next number to look for
	SCREENS[i].order = cnum; //assign the current order to it
	cnum++; //get ready for the next one
         AddScreenToWidget(SCREENS[i]);
      }
    }
  }

  //Now update the available/current screens in the UI
  ui->combo_availscreens->clear();
  ui->combo_cscreens->clear();
  for(int i=0; i<SCREENS.length(); i++){
    if(SCREENS[i].order<0){
      ui->combo_availscreens->addItem(SCREENS[i].ID);
    }else{
      ui->combo_cscreens->addItem(SCREENS[i].ID);
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
  //if(ui->list_screens->currentItem()==0){ ui->list_screens->setCurrentRow(0); }
  ScreenSelected(); //update buttons
  RRSettings::SaveScreens(SCREENS);
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

/*void MainUI::MoveScreenLeft(){
  QListWidgetItem *item = ui->list_screens->currentItem();
  if(item==0){ return; } //no selection
  //Get the current ID
  QString CID = item->whatsThis();
  //Now get the ID of the one on the left
  item = ui->list_screens->item( ui->list_screens->row(item)-1 );
  if(item == 0){ return; } //no item on the left (can't go left)
  QString LID = item->whatsThis(); //left ID
  //Adjust the order of the two screens
  for(int i=0; i<SCREENS.length(); i++){
    if(SCREENS[i].ID == CID){ SCREENS[i].order = SCREENS[i].order-1; }
    else if(SCREENS[i].ID==LID){ SCREENS[i].order = SCREENS[i].order+1; }
  }
  //Now run the command
  QStringList opts = currentOpts();
  LUtils::runCmd("xrandr", opts);
  //Now run the command
  //LUtils::runCmd("xrandr", QStringList() << "--output" << CID << "--left-of" << LID);
  QTimer::singleShot(500, this, SLOT(UpdateScreens()) );
}*/

/*void MainUI::MoveScreenRight(){
  QListWidgetItem *item = ui->list_screens->currentItem();
  if(item==0){ return; } //no selection
  //Get the current ID
  QString CID = item->whatsThis();
  //Now get the ID of the one on the left
  item = ui->list_screens->item( ui->list_screens->row(item)+1 );
  if(item == 0){ return; } //no item on the right (can't go right)
  QString RID = item->whatsThis(); //right ID
  //Adjust the order of the two screens
  for(int i=0; i<SCREENS.length(); i++){
    if(SCREENS[i].ID == RID){ SCREENS[i].order = SCREENS[i].order-1; }
    else if(SCREENS[i].ID==CID){ SCREENS[i].order = SCREENS[i].order+1; }
  }
  //Now run the command
  QStringList opts = currentOpts();
  LUtils::runCmd("xrandr", opts);
  QTimer::singleShot(500, this, SLOT(UpdateScreens()) );
}*/

void MainUI::DeactivateScreen(QString device){
  if(device.isEmpty()){ device = currentSelection(); }
  if(device.isEmpty()){ return; } //nothing found
  //Remove the screen from the settings
  for(int i=0; i<SCREENS.length(); i++){
    if(SCREENS[i].ID==device){ SCREENS.removeAt(i); break; }
  }
  //Now run the command
  QStringList opts = currentOpts();
  opts << "--output" << device << "--off";
  LUtils::runCmd("xrandr", opts);
  QTimer::singleShot(500, this, SLOT(UpdateScreens()) );
}

void MainUI::ActivateScreen(){
  //Assemble the command;
  QString ID = ui->combo_availscreens->currentText();
  QString DID = ui->combo_cscreens->currentText();
  QString loc = ui->combo_location->currentData().toString();
  if(ID.isEmpty() || DID.isEmpty() || loc.isEmpty()){ return; } //invalid inputs
  QStringList opts = currentOpts();
    opts << "--output" << ID << loc << DID <<"--auto";
  //qDebug() << "Activate Options:" << opts;
  LUtils::runCmd("xrandr", opts );
  QTimer::singleShot(500, this, SLOT(UpdateScreens()) );
}

void MainUI::ApplyChanges(){
  SyncBackend();
  //Now run the command
  RRSettings::Apply(SCREENS);
  //And update the UI and WM in a moment
  QTimer::singleShot(500, this, SLOT(UpdateScreens()) );
  QTimer::singleShot(1000, this, SLOT(RestartFluxbox()) );
}

void MainUI::SaveSettings(){
  SyncBackend();
  RRSettings::SaveScreens(SCREENS);
}

void MainUI::RestartFluxbox(){
  QProcess::startDetached("killall fluxbox");
}
