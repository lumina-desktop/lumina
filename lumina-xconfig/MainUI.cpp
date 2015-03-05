//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainUI.h"
#include "ui_MainUI.h"

#include <LuminaXDG.h>
#include <LuminaUtils.h>

#include <QTimer>

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI){
  ui->setupUi(this);
  loadIcons();
  //Fill the location list with the valid entries
  ui->combo_location->clear();
    ui->combo_location->addItem(tr("Right Of"), "--right-of");
    ui->combo_location->addItem(tr("Left Of"), "--left-of");

  connect(ui->push_close, SIGNAL(clicked()), this, SLOT(close()) );
  connect(ui->push_rescan, SIGNAL(clicked()), this, SLOT(UpdateScreens()) );
  connect(ui->push_activate, SIGNAL(clicked()), this, SLOT(ActivateScreen()) );
  connect(ui->tool_deactivate, SIGNAL(clicked()), this, SLOT(DeactivateScreen()) );
  connect(ui->tool_moveleft, SIGNAL(clicked()), this, SLOT(MoveScreenLeft()) );
  connect(ui->tool_moveright, SIGNAL(clicked()), this, SLOT(MoveScreenRight()) );
  connect(ui->list_screens, SIGNAL(itemSelectionChanged()),this, SLOT(ScreenSelected()) );
  QTimer::singleShot(0, this, SLOT(UpdateScreens()) );
}

MainUI::~MainUI(){
	
}

void MainUI::loadIcons(){
  this->setWindowIcon( LXDG::findIcon("preferences-system-windows-actions","") );
  ui->tool_deactivate->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_moveleft->setIcon( LXDG::findIcon("arrow-left","") );
  ui->tool_moveright->setIcon( LXDG::findIcon("arrow-right","") );
  ui->push_activate->setIcon( LXDG::findIcon("list-add","") );
  ui->push_rescan->setIcon( LXDG::findIcon("view-refresh","") );
  ui->push_close->setIcon( LXDG::findIcon("window-close","") );
}

void MainUI::UpdateScreens(){
  //First probe the server for current screens
  SCREENS.clear();
  QStringList info = LUtils::getCmdOutput("xrandr -q");
  ScreenInfo cscreen;
  for(int i=0; i<info.length(); i++){
    if(info[i].contains("connected") ){
      //qDebug() << "xrandr info:" << info[i];
      if(!cscreen.ID.isEmpty()){ 
	SCREENS << cscreen; //current screen finished - save it into the array
	cscreen = ScreenInfo(); //Now create a new structure      
      } 
      //qDebug() << "Line:" << info[i];
      QString dev = info[i].section(" ",0,0); //device ID
      //The device resolution can be either the 3rd or 4th output - check both
      QString devres = info[i].section(" ",2,2, QString::SectionSkipEmpty);
      if(!devres.contains("x")){ devres = info[i].section(" ",3,3,QString::SectionSkipEmpty); }
      if(!devres.contains("x")){ devres.clear(); }
      qDebug() << " - ID:" <<dev << "Current Geometry:" << devres;
      //qDebug() << " - Res:" << devres;
      if( !devres.contains("x") || !devres.contains("+") ){ devres.clear(); }
      //qDebug() << " - Res (modified):" << devres;
      if(info[i].contains(" disconnected ") && !devres.isEmpty() ){
	//Disable this device and restart (disconnected, but still attached to the X server)
	DeactivateScreen(dev);
	UpdateScreens();
	return;
      }else if( !devres.isEmpty() ){
	//Device that is connected and attached (has a resolution)
	qDebug() << "Create new Screen entry:" << dev << devres;
	cscreen.ID = dev;
	//Note: devres format: "<width>x<height>+<xoffset>+<yoffset>"
	cscreen.geom.setRect( devres.section("+",-2,-2).toInt(), devres.section("+",-1,-1).toInt(), devres.section("x",0,0).toInt(), devres.section("+",0,0).section("x",1,1).toInt() ); 
	
      }else if(info[i].contains(" connected")){
        //Device that is connected, but not attached
	qDebug() << "Create new Screen entry:" << dev << "none";
	cscreen.ID = dev;
	cscreen.order = -2; //flag this right now as a non-active screen
      }
    }else if( !cscreen.ID.isEmpty() && info[i].section("\t",0,0,QString::SectionSkipEmpty).contains("x")){
      //available resolution for a device
      cscreen.resList << info[i].section("\t",0,0,QString::SectionSkipEmpty);
    }
  }
  if(!cscreen.ID.isEmpty()){ SCREENS << cscreen; } //make sure to add the last screen to the array
  
  //Now go through the screens and arrange them in order from left->right in the UI
  bool found = true;
  int xoffset = 0; //start at 0
  int cnum = 0;
  ui->list_screens->clear();
  while(found){
    found = false; //make sure to break out if a screen is not found
    for(int i=0; i<SCREENS.length(); i++){
      if(SCREENS[i].order != -1){} //already evaluated - skip it
      else if(SCREENS[i].geom.x()==xoffset){
	found = true; //make sure to look for the next one
	xoffset = xoffset+SCREENS[i].geom.width(); //next number to look for
	SCREENS[i].order = cnum; //assign the currrent order to it
	cnum++; //get ready for the next one
	QListWidgetItem *it = new QListWidgetItem();
	  it->setTextAlignment(Qt::AlignCenter);
	  it->setText( SCREENS[i].ID+"\n  ("+QString::number(SCREENS[i].geom.width())+"x"+QString::number(SCREENS[i].geom.height())+")  " );
	  it->setWhatsThis(SCREENS[i].ID);
	ui->list_screens->addItem(it);
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
  ui->group_avail->setVisible( ui->combo_availscreens->count()>0 );
  ScreenSelected(); //update buttons
}

void MainUI::ScreenSelected(){
  QListWidgetItem *item = ui->list_screens->currentItem();
  if(item==0){
    //nothing selected
    ui->tool_deactivate->setEnabled(false);
    ui->tool_moveleft->setEnabled(false);
    ui->tool_moveright->setEnabled(false);
  }else{
    //Item selected
    ui->tool_deactivate->setEnabled(ui->list_screens->count()>1);
    ui->tool_moveleft->setEnabled(ui->list_screens->row(item) > 0);
    ui->tool_moveright->setEnabled(ui->list_screens->row(item) < (ui->list_screens->count()-1));
  }
}

void MainUI::MoveScreenLeft(){
  QListWidgetItem *item = ui->list_screens->currentItem();
  if(item==0){ return; } //no selection
  //Get the current ID
  QString CID = item->whatsThis();
  //Now get the ID of the one on the left
  item = ui->list_screens->item( ui->list_screens->row(item)-1 );
  if(item == 0){ return; } //no item on the left (can't go left)
  QString LID = item->whatsThis(); //left ID
  //Now run the command
  LUtils::runCmd("xrandr", QStringList() << "--output" << CID << "--left-of" << LID << "--auto");
  QTimer::singleShot(500, this, SLOT(UpdateScreens()) );
}

void MainUI::MoveScreenRight(){
  QListWidgetItem *item = ui->list_screens->currentItem();
  if(item==0){ return; } //no selection
  //Get the current ID
  QString CID = item->whatsThis();
  //Now get the ID of the one on the left
  item = ui->list_screens->item( ui->list_screens->row(item)+1 );
  if(item == 0){ return; } //no item on the right (can't go right)
  QString RID = item->whatsThis(); //right ID
  //Now run the command
  LUtils::runCmd("xrandr", QStringList() << "--output" << CID << "--right-of" << RID << "--auto");
  QTimer::singleShot(500, this, SLOT(UpdateScreens()) );
}

void MainUI::DeactivateScreen(QString device){
  if(device.isEmpty()){
    //Get the currently selected device
    QListWidgetItem *item = ui->list_screens->currentItem();
    if(item==0){ return; } //no selection
    //Get the current ID
    device = item->whatsThis();
  }
  if(device.isEmpty()){ return; } //nothing found
  LUtils::runCmd("xrandr", QStringList() << "--output" << device << "--off");
  QTimer::singleShot(500, this, SLOT(UpdateScreens()) );
}

void MainUI::ActivateScreen(){
  //Assemble the command;
  QString ID = ui->combo_availscreens->currentText();
  QString DID = ui->combo_cscreens->currentText();
  QString loc = ui->combo_location->currentData().toString();
  if(ID.isEmpty() || DID.isEmpty() || loc.isEmpty()){ return; } //invalid inputs
  LUtils::runCmd("xrandr", QStringList() << "--output" << ID << loc << DID <<"--auto");
  QTimer::singleShot(500, this, SLOT(UpdateScreens()) );
}
