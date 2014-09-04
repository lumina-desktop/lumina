//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainUI.h"
#include "ui_MainUI.h"

#include <LuminaX11.h>

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI){
  ui->setupUi(this); //load the designer file
  cpic = QPixmap::grabWindow(QApplication::desktop()->winId()); //initial screenshot
  ppath = QDir::homePath();
  QWidget *spacer = new QWidget();
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ui->toolBar->insertWidget(ui->actionNew, spacer);
  //Setup the icons
  this->setWindowIcon( LXDG::findIcon("camera-web","") );
  ui->actionSave->setIcon( LXDG::findIcon("document-save","") );
  ui->actionQuit->setIcon( LXDG::findIcon("application-exit","") );
  ui->actionNew->setIcon( LXDG::findIcon("camera-web","") );
  //Setup the connections
  connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveScreenshot()) );
  connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(closeApplication()) );
  connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(startScreenshot()) );
  ui->radio_window->setChecked(true);

  this->show();
  ui->label_screenshot->setPixmap( cpic.scaled(ui->label_screenshot->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
}

MainUI::~MainUI(){}
	
//==============
//  PRIVATE SLOTS
//==============
void MainUI::saveScreenshot(){
  QString filepath = QFileDialog::getSaveFileName(this, tr("Save Screenshot"), ppath, tr("PNG Files (*.png);;AllFiles (*)") );
  if(filepath.isEmpty()){ return; }
  if(!filepath.endsWith(".png")){ filepath.append(".png"); }
  cpic.save(filepath, "png");
  ppath = filepath;
}

void MainUI::startScreenshot(){
  if( !getWindow() ){ return; }
  this->hide();
  QTimer::singleShot(50+ui->spin_delay->value()*1000, this, SLOT(getPixmap()));
}

bool MainUI::getWindow(){
  //Use this function to set cwin
  cwin = 0;
  if(ui->radio_window->isChecked()){
    //Use xprop to get the desired window from the user
    QList<WId> wins = LX11::WindowList();
    wins.removeAll(this->winId()); //don't show this window
    QStringList names;
    for(int i=0; i<wins.length(); i++){
      names << LX11::WindowClass(wins[i])+" ("+LX11::WindowName(wins[i])+")";
    }
    bool ok = false;
    QString info = QInputDialog::getItem(this, tr("Select Window"), tr("Window:"), names, 0, false, &ok);
    if(!ok || names.indexOf(info)<0){ return false; } //cancelled
    cwin = wins[ names.indexOf(info) ];
  }
  return true;
}

void MainUI::getPixmap(){
  if(cwin==0){
    //Grab the whole screen
    cpic = QPixmap::grabWindow(QApplication::desktop()->winId());
  }else{
    //Grab just the designated window
    cpic = QPixmap::grabWindow(cwin);
  }
  this->show();
  //Now display the pixmap on the label as well
  ui->label_screenshot->setPixmap( cpic.scaled(ui->label_screenshot->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
}