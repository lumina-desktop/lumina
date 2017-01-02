//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainUI.h"
#include "ui_MainUI.h"

#include <LuminaX11.h>
#include <QMessageBox>


MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI){
  ui->setupUi(this); //load the designer file
  mousegrabbed = false;
  XCB = new LXCB();
  IMG = new ImageEditor(this);
  ui->scrollArea->setWidget(IMG);
  ppath = QDir::homePath();
  ui->label_zoom_percent->setMinimumWidth( ui->label_zoom_percent->fontMetrics().width("200%") );
  setupIcons();
  ui->spin_monitor->setMaximum(QApplication::desktop()->screenCount());
  if(ui->spin_monitor->maximum()<2){
    ui->spin_monitor->setEnabled(false);
    ui->radio_monitor->setEnabled(false);
  }	  
  scaleTimer = new QTimer(this);
    scaleTimer->setSingleShot(true);
    scaleTimer->setInterval(200); //~1/5 second

  //Setup the connections
  connect(ui->tool_save, SIGNAL(clicked()), this, SLOT(saveScreenshot()) );
  connect(ui->actionSave_As, SIGNAL(triggered()), this, SLOT(saveScreenshot()) );
  connect(ui->tool_quicksave, SIGNAL(clicked()), this, SLOT(quicksave()) );
  connect(ui->actionQuick_Save, SIGNAL(triggered()), this, SLOT(quicksave()) );
  connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(closeApplication()) );
  connect(ui->push_snap, SIGNAL(clicked()), this, SLOT(startScreenshot()) );
  connect(ui->actionTake_Screenshot, SIGNAL(triggered()), this, SLOT(startScreenshot()) );
  connect(ui->tool_crop, SIGNAL(clicked()), IMG, SLOT(cropImage()) );
  connect(IMG, SIGNAL(selectionChanged(bool)), this, SLOT(imgselchanged(bool)) );
  connect(IMG, SIGNAL(scaleFactorChanged(int)), this, SLOT(imgScalingChanged(int)) );
  connect(ui->slider_zoom, SIGNAL(valueChanged(int)),  this, SLOT(sliderChanged()) );
  connect(scaleTimer, SIGNAL(timeout()), this, SLOT(imgScalingChanged()) );

  settings = new QSettings("lumina-desktop", "lumina-screenshot",this);
  if(settings->value("screenshot-target", "window").toString() == "window") {
	ui->radio_window->setChecked(true);
  }else{
	ui->radio_all->setChecked(true);
  }
  ui->spin_delay->setValue(settings->value("screenshot-delay", 0).toInt());

  ui->tool_resize->setVisible(false); //not implemented yet
  this->show();
  IMG->setDefaultSize(ui->scrollArea->maximumViewportSize());
  IMG->LoadImage( QApplication::screens().at(0)->grabWindow(QApplication::desktop()->winId()).toImage() ); //initial screenshot
  lastScreenShot = QDateTime::currentDateTime();
  //ui->label_screenshot->setPixmap( cpic.scaled(ui->label_screenshot->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
}

MainUI::~MainUI(){}

void MainUI::setupIcons(){
  //Setup the icons
  ui->tool_save->setIcon( LXDG::findIcon("document-save","") );
  ui->tool_quicksave->setIcon( LXDG::findIcon("document-edit","") );
  ui->actionSave_As->setIcon( LXDG::findIcon("document-save-as","") );
  ui->actionQuick_Save->setIcon( LXDG::findIcon("document-save","") );
  ui->actionClose->setIcon( LXDG::findIcon("application-exit","") );
  ui->push_snap->setIcon( LXDG::findIcon("camera-web","") );
  ui->actionTake_Screenshot->setIcon( LXDG::findIcon("camera-web","") );
  ui->tool_crop->setIcon( LXDG::findIcon("transform-crop","") );
  ui->tool_resize->setIcon( LXDG::findIcon("transform-scale","") );
  //ui->actionEdit->setIcon( LXDG::findIcon("applications-graphics","") );
}

void MainUI::showSaveError(QString path){
  QMessageBox::warning(this, tr("Could not save screenshot"), tr("The screenshot could not be saved. Please check directory permissions or pick a different directory")+"\n\n"+path);
}
//==============
//  PRIVATE SLOTS
//==============
void MainUI::saveScreenshot(){
  if(mousegrabbed){ return; }
  QString filepath = QFileDialog::getSaveFileName(this, tr("Save Screenshot"), ppath+"/"+QString( "Screenshot-%1.png" ).arg( lastScreenShot.toString("yyyy-MM-dd-hh-mm-ss")), tr("PNG Files (*.png);;AllFiles (*)") );
  if(filepath.isEmpty()){ return; }
  if(!filepath.endsWith(".png")){ filepath.append(".png"); }
  if( !IMG->image().save(filepath, "png") ){
    showSaveError(filepath);
  }else{
    ppath = filepath.section("/",0,-2); //just the directory
  }
}

void MainUI::quicksave(){
  if(mousegrabbed){ return; }
    QString savedir = QDir::homePath()+"/";
    if(QFile::exists(savedir + "Pictures/")){ savedir.append("Pictures/"); }
    else if(QFile::exists(savedir + "Images/")){ savedir.append("Images/"); }

    QString path = savedir + QString( "Screenshot-%1.png" ).arg( lastScreenShot.toString("yyyy-MM-dd-hh-mm-ss") );
    if(IMG->image().save(path, "png") ){
      QProcess::startDetached("lumina-open \""+path+"\"");
    }else{
      showSaveError(path);
    }
}

void MainUI::startScreenshot(){
  if(mousegrabbed){ return; }
  lastgeom = this->geometry();
  if( !getWindow() ){ return; }
  this->hide();
  QTimer::singleShot(50+ui->spin_delay->value()*1000, this, SLOT(getPixmap()));
}

void MainUI::imgselchanged(bool hassel){
  ui->tool_crop->setEnabled(hassel);
  ui->tool_resize->setEnabled(hassel);
}

void MainUI::imgScalingChanged(int percent){
  //qDebug() << "Scale Changed:" << percent;
  if(percent<0){
    //Changed by user interaction
    IMG->setScaling(ui->slider_zoom->value());
  }else{
    ui->slider_zoom->setValue(percent);
  }
  ui->label_zoom_percent->setText( QString::number(ui->slider_zoom->value())+"%");
}

void MainUI::sliderChanged(){
  ui->label_zoom_percent->setText( QString::number(ui->slider_zoom->value())+"%");
  scaleTimer->start();
}

bool MainUI::getWindow(){
  //Use this function to set cwin
  cwin = 0;
  //Save all the current settings for later
  settings->setValue("screenshot-delay", ui->spin_delay->value());
  if(ui->radio_window->isChecked()){
    settings->setValue("screenshot-target", "window");
    this->grabMouse( QCursor(Qt::CrossCursor) );
    mousegrabbed = true;
    this->centralWidget()->setEnabled(false);
    //this->hide();
    return false; //wait for the next click to continue
  }else if(ui->radio_monitor->isChecked()){
    //will auto-grab the proper monitor later
  }else{
    settings->setValue("screenshot-target", "desktop");
  }
  return true;
}

void MainUI::getPixmap(){
  QScreen *scrn = QApplication::screens().at(0);
  QPixmap cpic;
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
  this->setGeometry(lastgeom);
  lastScreenShot = QDateTime::currentDateTime();
  //Now display the pixmap on the label as well
  IMG->LoadImage( cpic.toImage() );
}

void MainUI::mouseReleaseEvent(QMouseEvent *ev){
  if(mousegrabbed){
    mousegrabbed = false;
    this->centralWidget()->setEnabled(true);
    this->releaseMouse();
    //In the middle of selecting a window to take a screenshot
    //  Get the window underneath the mouse click and take the screenshot
    QList<WId> wins = XCB->WindowList();
    QList<WId> stack = XCB->WM_Get_Client_List(true);
    cwin = 0;
    //qDebug() << "Try to select window:" << ev->globalPos(); 
    //for(int i=0; i<stack.length(); i++){
    for(int i=stack.length()-1; i>=0 && cwin==0; i--){ //work top->bottom in the stacking order
      if(!wins.contains(stack[i])){ continue; }
      if( XCB->WindowGeometry(stack[i], true).contains(ev->globalPos()) && XCB->WindowState(stack[i])!=LXCB::INVISIBLE ){ 
        qDebug() << "Found Window:" << i << XCB->WindowClass(stack[i]);
        cwin = stack[i]; 
      }
    }
    qDebug() << " - Got window:" << cwin;
    if(cwin==this->winId()){  return; } //cancelled
    this->hide();
    QTimer::singleShot(50+ui->spin_delay->value()*1000, this, SLOT(getPixmap()));
  }else{
    QMainWindow::mouseReleaseEvent(ev); //normal processing
  }
}

void MainUI::resizeEvent(QResizeEvent*){
  IMG->setDefaultSize( ui->scrollArea->maximumViewportSize() );
}
