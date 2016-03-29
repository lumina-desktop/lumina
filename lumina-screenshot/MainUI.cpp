//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainUI.h"
#include "ui_MainUI.h"

#include <LuminaX11.h>

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI){
  ui->setupUi(this); //load the designer file
  XCB = new LXCB();
  cpic = QApplication::screens().at(0)->grabWindow(QApplication::desktop()->winId()); //initial screenshot
  ppath = QDir::homePath();
  QWidget *spacer = new QWidget();
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ui->toolBar->insertWidget(ui->actionNew, spacer);
	
  setupIcons();
  ui->spin_monitor->setMaximum(QApplication::desktop()->screenCount());
  if(ui->spin_monitor->maximum()<2){
    ui->spin_monitor->setEnabled(false);
    ui->radio_monitor->setEnabled(false);
  }	  

  //Setup the connections
  connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveScreenshot()) );
  connect(ui->actionquicksave, SIGNAL(triggered()), this, SLOT(quicksave()) );
  connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(closeApplication()) );
  connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(startScreenshot()) );
  connect(ui->actionEdit, SIGNAL(triggered()), this, SLOT(editScreenshot()) );

  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QDir::homePath()+"/.lumina");
  settings = new QSettings("LuminaDE", "lumina-screenshot",this);

  if(settings->value("screenshot-target", "window").toString() == "window") {
	ui->radio_window->setChecked(true);
  }else{
	ui->radio_all->setChecked(true);
  }

  ui->spin_delay->setValue(settings->value("screenshot-delay", 0).toInt());

  this->show();
  ui->label_screenshot->setPixmap( cpic.scaled(ui->label_screenshot->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
}

MainUI::~MainUI(){}

void MainUI::setupIcons(){
  //Setup the icons
  ui->actionSave->setIcon( LXDG::findIcon("document-save","") );
  ui->actionquicksave->setIcon( LXDG::findIcon("document-save","") );
  ui->actionQuit->setIcon( LXDG::findIcon("application-exit","") );
  ui->actionNew->setIcon( LXDG::findIcon("camera-web","") );	
  ui->actionEdit->setIcon( LXDG::findIcon("applications-graphics","") );
}

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
void MainUI::quicksave(){
    QString savedir = QDir::homePath() + "/Pictures/";
    QString path = savedir + QString( "Screenshot-%1.png" ).arg( QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss") );
    cpic.save(path, "png");
}

void MainUI::editScreenshot(){
    QString tmppath = QString("/tmp/screenshot.png");
    cpic.save(tmppath, "png");
   QProcess::startDetached("lumina-open /tmp/screenshot.png");
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
    settings->setValue("screenshot-target", "window");
    //Use xprop to get the desired window from the user
    QList<WId> wins = XCB->WindowList();
    wins.removeAll(this->winId()); //don't show this window
    QStringList names;
    for(int i=0; i<wins.length(); i++){
      names << XCB->WindowClass(wins[i])+" ("+XCB->WindowName(wins[i])+")";
    }
    bool ok = false;
    QString info = QInputDialog::getItem(this, tr("Select Window"), tr("Window:"), names, 0, false, &ok);
    if(!ok || names.indexOf(info)<0){ return false; } //cancelled
    cwin = wins[ names.indexOf(info) ];
  }else if(ui->radio_monitor->isChecked()){
    
  }else{
    settings->setValue("screenshot-target", "desktop");
  }
  settings->setValue("screenshot-delay", ui->spin_delay->value());
  return true;
}

void MainUI::getPixmap(){
  QScreen *scrn = QApplication::screens().at(0);
  if( (cwin==0 && ui->radio_window->isChecked() ) || ui->radio_all->isChecked() ){
    //Grab the whole screen
    cpic = scrn->grabWindow(QApplication::desktop()->winId());
  }else if(cwin==0 && ui->radio_monitor->isChecked()){
    QRect geom = QApplication::desktop()->screenGeometry(ui->spin_monitor->value()-1);
    cpic = scrn->grabWindow(QApplication::desktop()->winId(), geom.x(), geom.y(), geom.width(), geom.height() );
  }else{
    //Grab just the designated window
    if(ui->check_frame->isChecked()){
      QRect geom = XCB->WindowGeometry(cwin, true); //include the frame
      cpic = scrn->grabWindow(QApplication::desktop()->winId(), geom.x(), geom.y(), geom.width(), geom.height() );
    }else{
      cpic = scrn->grabWindow(cwin);
    }
  }
  this->show();
  //Now display the pixmap on the label as well
  ui->label_screenshot->setPixmap( cpic.scaled(ui->label_screenshot->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
}
