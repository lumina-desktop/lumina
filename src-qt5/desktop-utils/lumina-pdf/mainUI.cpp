//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "mainUI.h"
#include <QPainter>
#include <QImage>
#include <QSize>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>

#include <LuminaXDG.h>

MainUI::MainUI() : QPrintPreviewDialog(0, Qt::Window){

  //this->addStatusBar();
  
  connect(this, SIGNAL(paintRequested(QPrinter*)), this, SLOT(paintOnWidget(QPrinter*)) );
  DOC = 0;
}

MainUI::~MainUI(){

}

void MainUI::loadFile(QString path){
  if(DOC!=0){
    //Clear out the old document first
    delete DOC;
  }
  if(!QFile::exists(path) || path.isEmpty() ){QApplication::exit(1); }
  DOC = Poppler::Document::load(path);
  if(DOC!=0){ this->setWindowTitle(DOC->title()); }
  if(this->windowTitle().isEmpty()){ this->setWindowTitle(path.section("/",-1)); }
  this->setWindowIcon( LXDG::findIcon("application-pdf","unknown"));
  //Setup the Document
  Poppler::Page *PAGE = DOC->page(0);
  if(PAGE!=0){
    this->printer()->setPageSize( QPageSize(PAGE->pageSize(), QPageSize::Point) ); 
    switch(PAGE->orientation()){
	case Poppler::Page::Landscape:
	  this->printer()->setOrientation(QPrinter::Landscape); break;
	default:
	  this->printer()->setOrientation(QPrinter::Portrait);
    }
    delete PAGE;
  }

}

void MainUI::done(int val){
  //This is automatically called with value=1 when the "print()" function finishes
  //Otherwise close down the application
  if(val==0){ QApplication::exit(0); }
}

void MainUI::paintOnWidget(QPrinter *PRINTER){
  if(DOC==0){ return; }
  int pages = DOC->numPages();
  int firstpage = 0;
  //qDebug() << "Start Rendering PDF:" << PRINTER->fromPage() << PRINTER->toPage();
  if(PRINTER->fromPage() != PRINTER->toPage() || PRINTER->fromPage()!=0){
    firstpage = PRINTER->fromPage() - 1;
    pages = PRINTER->toPage();
  }
  qDebug() << " - Generating Pages:" << firstpage << pages;
  //Now start painting all the pages onto the widget
  QRectF size = PRINTER->pageRect(QPrinter::DevicePixel);
  QSize DPI(PRINTER->resolution(),PRINTER->resolution());
  QPainter painter(PRINTER);
  Poppler::Page *PAGE = 0;
  QMessageBox wait(QMessageBox::NoIcon, tr("Opening PDF..."), QString(tr("Preparing Document (%1 pages)")).arg(QString::number(pages)), QMessageBox::Abort, this);
    wait.setInformativeText(" "); //Make sure the window is the right size before showing it
    wait.setStandardButtons(QMessageBox::Abort); //make sure that no buttons are used
    wait.show();
    for(int i=firstpage; i<pages && wait.isVisible(); i++){
      wait.setInformativeText( QString(tr("Loading Page: %1")).arg(i+1) );
      //qDebug() << "Loading Page:" << i;
      QApplication::processEvents();
      //Now paint this page on the printer
      if(PAGE!=0){ delete PAGE; PRINTER->newPage(); } //this is the start of the next page (not needed for first)
      PAGE = DOC->page(i);
      if(PAGE!=0){
        painter.drawImage(0,0,PAGE->renderToImage(DPI.width(), DPI.height()).scaled(size.width(), size.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
      }else{
        painter.drawImage(0,0,QImage()); 
      }
      QApplication::processEvents();
    }
  if(PAGE!=0){ delete PAGE; }
  wait.close();
}

void MainUI::OpenNewFile(){
  //Prompt for a file

  //Now Open it
  
}
