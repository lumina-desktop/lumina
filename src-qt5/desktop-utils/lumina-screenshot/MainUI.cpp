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
#include <QClipboard>

MainUI::MainUI()
  : QMainWindow(), ui(new Ui::MainUI),
    mousegrabbed(false),
    picSaved(false),
    closeOnSave(false)
{
  ui->setupUi(this); //load the designer file
  XCB = new LXCB();
  IMG = new ImageEditor(this);
  ui->scrollArea->setWidget(IMG);
  areaOverlay = 0;
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
  tabbar = new QTabBar(this);
    tabbar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
  ui->tabLayout->insertWidget(0,tabbar, Qt::AlignLeft | Qt::AlignBottom);
    tabbar->addTab(LXDG::findIcon("view-preview",""), tr("View"));
    tabbar->addTab(LXDG::findIcon("preferences-other",""), tr("Settings"));
  ui->stackedWidget->setCurrentWidget(ui->page_current);
  //Add a spacer in the Toolbar
  QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
  ui->toolBar->insertWidget(ui->actionClose, spacer);
  //Setup the connections
  //connect(ui->tool_save, SIGNAL(clicked()), this, SLOT(saveScreenshot()) );
  connect(ui->actionSave_As, SIGNAL(triggered()), this, SLOT(saveScreenshot()) );
  connect(ui->tool_quicksave, SIGNAL(clicked()), this, SLOT(quicksave()) );
  connect(ui->actionQuick_Save, SIGNAL(triggered()), this, SLOT(quicksave()) );
  connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(closeApplication()) );
  //connect(ui->push_snap, SIGNAL(clicked()), this, SLOT(startScreenshot()) );
  connect(ui->actionTake_Screenshot, SIGNAL(triggered()), this, SLOT(startScreenshot()) );
  connect(ui->tool_crop, SIGNAL(clicked()), IMG, SLOT(cropImage()) );
  connect(ui->tool_copy_to_clipboard, SIGNAL(clicked()), this, SLOT(copyToClipboard()) );
  connect(IMG, SIGNAL(selectionChanged(bool)), this, SLOT(imgselchanged(bool)) );
  connect(IMG, SIGNAL(scaleFactorChanged(int)), this, SLOT(imgScalingChanged(int)) );
  connect(ui->slider_zoom, SIGNAL(valueChanged(int)),  this, SLOT(sliderChanged()) );
  connect(scaleTimer, SIGNAL(timeout()), this, SLOT(imgScalingChanged()) );
  connect(tabbar, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)) );
  connect(ui->check_show_popups, SIGNAL(toggled(bool)), this, SLOT(showPopupsChanged(bool)) );
  settings = new QSettings("lumina-desktop", "lumina-screenshot",this);
  QString opt = settings->value("screenshot-target", "window").toString();
  if( opt == "window") {ui->radio_window->setChecked(true); }
  else if(opt=="area"){ ui->radio_area->setChecked(true); }
  else{ ui->radio_all->setChecked(true); }
  ui->spin_delay->setValue(settings->value("screenshot-delay", 0).toInt());
  ui->check_show_popups->setChecked( settings->value("showPopupWarnings",true).toBool() );

  ui->tool_resize->setVisible(false); //not implemented yet
  this->show();
  IMG->setDefaultSize(ui->scrollArea->maximumViewportSize());
  IMG->LoadImage( QApplication::screens().at(0)->grabWindow(QApplication::desktop()->winId()).toImage() ); //initial screenshot
  lastScreenShot = QDateTime::currentDateTime();
  //ui->label_screenshot->setPixmap( cpic.scaled(ui->label_screenshot->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
}

MainUI::~MainUI(){
  if(areaOverlay!=0){ areaOverlay->deleteLater(); }
}

void MainUI::setupIcons(){
  //Setup the icons
  ui->tool_quicksave->setIcon( LXDG::findIcon("document-edit","") );
  ui->actionSave_As->setIcon( LXDG::findIcon("document-save-as","") );
  ui->actionQuick_Save->setIcon( LXDG::findIcon("document-save","") );
  ui->actionClose->setIcon( LXDG::findIcon("application-exit","") );
  ui->tool_copy_to_clipboard->setIcon( LXDG::findIcon("insert-image","") );
  ui->actionTake_Screenshot->setIcon( LXDG::findIcon("camera-web","") );
  ui->tool_crop->setIcon( LXDG::findIcon("transform-crop","") );
  ui->tool_resize->setIcon( LXDG::findIcon("transform-scale","") );
  this->setWindowIcon( LXDG::findIcon("camera-web","") );
}

void MainUI::showSaveError(QString path){
  QMessageBox::warning(this, tr("Could not save screenshot"), tr("The screenshot could not be saved. Please check directory permissions or pick a different directory")+"\n\n"+path);
}

QRect MainUI::pointsToRect(QPoint pt1, QPoint pt2){
  QRect rec;
  if(pt1.x() < pt2.x()){ rec.setLeft(pt1.x()); rec.setRight(pt2.x()); }
  else{ rec.setLeft(pt2.x()); rec.setRight(pt1.x()); }
  if(pt1.y() < pt2.y()){ rec.setTop(pt1.y()); rec.setBottom(pt2.y()); }
  else{ rec.setTop(pt2.y()); rec.setBottom(pt1.y()); }
  return rec;
}

//==============
//  PRIVATE SLOTS
//==============
void MainUI::saveScreenshot(){
  if(mousegrabbed){ return; }
  QString filepath = QFileDialog::getSaveFileName(this, tr("Save Screenshot"), ppath+"/"+QString( "Screenshot-%1.png" ).arg( lastScreenShot.toString("yyyy-MM-dd-hh-mm-ss")), tr("PNG Files (*.png);;AllFiles (*)") );
  if(filepath.isEmpty()){
    closeOnSave = false;
    return;
  }
  if(!filepath.endsWith(".png")){ filepath.append(".png"); }
  if( !IMG->image().save(filepath, "png") ){
    closeOnSave = false;
    showSaveError(filepath);
  }else{
    picSaved = true;
    ppath = filepath.section("/",0,-2); //just the directory
    if (closeOnSave) {
      // We came here from close, now we need to close *after* handling
      // the current screen event.
      QTimer::singleShot(0, this, SLOT(close()));
    }
  }
}

void MainUI::quicksave(){
  if(mousegrabbed){ return; }
    QString savedir = QDir::homePath()+"/";
    if(QFile::exists(savedir + "Pictures/")){ savedir.append("Pictures/"); }
    else if(QFile::exists(savedir + "Images/")){ savedir.append("Images/"); }

    QString path = savedir + QString( "Screenshot-%1.png" ).arg( lastScreenShot.toString("yyyy-MM-dd-hh-mm-ss") );
    if(IMG->image().save(path, "png") ){
      picSaved = true;
      QProcess::startDetached("lumina-open -select \""+path+"\"");
    }else{
      showSaveError(path);
    }

}
void MainUI::copyToClipboard(){
  qDebug() << "Copy Image to clipboard";
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setImage(IMG->image());
  qDebug() << " - Success:" << !clipboard->image().isNull();
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

void MainUI::tabChanged(int tab){
  if(tab==0){ ui->stackedWidget->setCurrentWidget(ui->page_current); }
  else{ ui->stackedWidget->setCurrentWidget(ui->page_settings); }
  ui->frame_modify->setVisible(tab==0);
}

void MainUI::showPopupsChanged(bool show){
  settings->setValue("showPopupWarnings", show);
}

bool MainUI::getWindow(){
  //Use this function to set cwin
  cwin = 0;
  snapArea = QRect(); //clear this too
  //Save all the current settings for later
  settings->setValue("screenshot-delay", ui->spin_delay->value());
  if(ui->radio_window->isChecked()){
    settings->setValue("screenshot-target", "window");
    this->grabMouse( QCursor(Qt::CrossCursor) );
    mousegrabbed = true;
    this->centralWidget()->setEnabled(false);
    //this->hide();
    return false; //wait for the next click to continue
  }else  if(ui->radio_area->isChecked()){
    settings->setValue("screenshot-target", "area");
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
  }else if(cwin==0 && ui->radio_area->isChecked()){
    //Grab the section of the screen which was selected
    cpic = scrn->grabWindow(QApplication::desktop()->winId(), snapArea.x(), snapArea.y(), snapArea.width(), snapArea.height() );
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
  picSaved = false;
  IMG->LoadImage( cpic.toImage() );
  tabbar->setCurrentIndex(0);
}
void MainUI::mousePressEvent(QMouseEvent *ev){
  if(mousegrabbed && ui->radio_area->isChecked()){
    pt_click = ev->globalPos();
    if(areaOverlay == 0){
      areaOverlay =  new QWidget(0, Qt::Window | Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
      areaOverlay->setWindowOpacity(0.5);
      areaOverlay->setStyleSheet("background-color: rgba(150,150,150,120)");
    }
  }
  QMainWindow::mouseMoveEvent(ev);
}

void MainUI::mouseMoveEvent(QMouseEvent *ev){
  if(mousegrabbed && ui->radio_area->isChecked()){
    //Not used yet - do something to paint the area so the user can see which area is selected
    QRect area = pointsToRect(pt_click, ev->globalPos());
    areaOverlay->setGeometry(area);
    areaOverlay->show();
  }else{
    QMainWindow::mouseMoveEvent(ev);
  }
}

void MainUI::mouseReleaseEvent(QMouseEvent *ev){
  if(mousegrabbed){
    mousegrabbed = false;
    this->centralWidget()->setEnabled(true);
    this->releaseMouse();
    if(ui->radio_area->isChecked()){
      //Need to determind the rectange which covers the area selected
      areaOverlay->hide();
      snapArea = pointsToRect(pt_click, ev->globalPos());
    }else{
      //In the middle of selecting a window to take a screenshot
      //  Get the window underneath the mouse click and take the screenshot
      QList<WId> wins = XCB->WindowList();
      QList<WId> stack = XCB->WM_Get_Client_List(true);
      cwin = 0;
      //qDebug() << "Try to select window:" << ev->globalPos(); 
      for(int i=stack.length()-1; i>=0 && cwin==0; i--){ //work top->bottom in the stacking order
        if(!wins.contains(stack[i])){ continue; }
        if( XCB->WindowGeometry(stack[i], true).contains(ev->globalPos()) && XCB->WindowState(stack[i])!=LXCB::INVISIBLE ){ 
          //qDebug() << "Found Window:" << i << XCB->WindowClass(stack[i]);
          cwin = stack[i]; 
        }
      }
      //qDebug() << " - Got window:" << cwin;
      if(cwin==this->winId()){  return; } //cancelled
    }
    this->hide();
    QTimer::singleShot(300+ui->spin_delay->value()*1000, this, SLOT(getPixmap()));
  }else{
    QMainWindow::mouseReleaseEvent(ev); //normal processing
  }
}

void MainUI::resizeEvent(QResizeEvent*){
  IMG->setDefaultSize( ui->scrollArea->maximumViewportSize() );
}

void MainUI::closeEvent(QCloseEvent *ev){
  //qDebug() << "Close Event:" << ui->check_show_popups->isChecked() << picSaved;
  if(ui->check_show_popups->isChecked() && !picSaved){
    //Ask what to do about the unsaved changed
    const int messageRet = QMessageBox::warning(this, tr("Unsaved Screenshot"),
				    tr("The current screenshot has not been saved yet. Do you want to save or discard your changes?"),
				    QMessageBox::Discard | QMessageBox::Save |QMessageBox::Cancel, QMessageBox::Cancel);
    switch (messageRet) {
    case QMessageBox::Discard:
      // Just close, we don't care about the file.
      break;
    case QMessageBox::Save:
      closeOnSave = true;
      saveScreenshot();
      // fall through
    case QMessageBox::Cancel:
      ev->ignore();
      return;
    }
  }
  QMainWindow::closeEvent(ev);
}
