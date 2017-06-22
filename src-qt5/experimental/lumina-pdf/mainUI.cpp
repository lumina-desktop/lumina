//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "mainUI.h"
#include "ui_mainUI.h"

#include <QPainter>
#include <QImage>
#include <QSize>
#include <QFileDialog>
#include <QInputDialog>
#include <QDebug>
#include <QApplication>
#include <QScreen>

#include <LuminaXDG.h>

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI()){
  ui->setupUi(this);

  this->setWindowTitle(tr("Lumina PDF Viewer"));
  this->setWindowIcon( LXDG::findIcon("application-pdf","unknown"));

  lastdir = QDir::homePath();
  Printer = new QPrinter();
  WIDGET = new QPrintPreviewWidget(Printer,this);
  this->setCentralWidget(WIDGET);
  connect(WIDGET, SIGNAL(paintRequested(QPrinter*)), this, SLOT(paintOnWidget(QPrinter*)) );
  DOC = 0;

  PrintDLG = new QPrintDialog(this);
  connect(PrintDLG, SIGNAL(accepted(QPrinter*)), this, SLOT(paintOnWidget(QPrinter*)) ); //Can change to PaintToPrinter() later

  //Create the other interface widgets
  progress = new QProgressBar(this);
    progress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    progress->setFormat("%v/%m (%p%)"); // [current]/[total]
  progAct = ui->toolBar->addWidget(progress);
  progAct->setVisible(false);
  //Put the various actions into logical groups
  QActionGroup *tmp = new QActionGroup(this);
    tmp->setExclusive(true);
    tmp->addAction(ui->actionFit_Width);
    tmp->addAction(ui->actionFit_Page);
  ui->actionFit_Page->setChecked(true);

  tmp = new QActionGroup(this);
    tmp->setExclusive(true);
    tmp->addAction(ui->actionSingle_Page);
    tmp->addAction(ui->actionDual_Pages);
    tmp->addAction(ui->actionAll_Pages);
  ui->actionSingle_Page->setChecked(true);

  //Connect up the buttons
  connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(close()) );
  connect(ui->actionPrint, SIGNAL(triggered()), PrintDLG, SLOT(open()) );
  connect(ui->actionFit_Width, SIGNAL(triggered()), WIDGET, SLOT(fitToWidth()) );
  connect(ui->actionFit_Page, SIGNAL(triggered()), WIDGET, SLOT(fitInView()) );
  connect(ui->actionOpen_PDF, SIGNAL(triggered()), this, SLOT(OpenNewFile()) );
  connect(ui->actionSingle_Page, SIGNAL(triggered()), WIDGET, SLOT(setSinglePageViewMode()) );
  connect(ui->actionDual_Pages, SIGNAL(triggered()), WIDGET, SLOT(setFacingPagesViewMode()) );
  connect(ui->actionAll_Pages, SIGNAL(triggered()), WIDGET, SLOT(setAllPagesViewMode()) );

  //Setup all the icons
  ui->actionPrint->setIcon( LXDG::findIcon("document-print",""));
  ui->actionClose->setIcon( LXDG::findIcon("window-close",""));
  ui->actionFit_Width->setIcon(LXDG::findIcon("arrow-expand-all",""));
  ui->actionFit_Page->setIcon(LXDG::findIcon("zoom-fit-best",""));
  ui->actionOpen_PDF->setIcon(LXDG::findIcon("document-open",""));
  ui->actionSingle_Page->setIcon(LXDG::findIcon("view-preview",""));
  ui->actionDual_Pages->setIcon(LXDG::findIcon("format-view-agenda",""));
  ui->actionAll_Pages->setIcon(LXDG::findIcon("format-view-grid-small",""));
}

MainUI::~MainUI(){

}

void MainUI::loadFile(QString path){

  if(!QFile::exists(path) || path.isEmpty() ){ return; }
  Poppler::Document *TDOC = Poppler::Document::load(path);
  if(TDOC==0){
    qDebug() << "Could not open file:" << path;
    return;
  }else if(TDOC->isLocked()){
    //Prompt for password to unlock the document
    QString pass = "";
    bool ok = true; //flag this to go into the loop the first time (if password prompt is cancelled - this becomes false)
    while( (ok ? true : !TDOC->unlock(QByteArray(), pass.toLocal8Bit())) ){ //make sure the unlock function is only called when ok is true
      pass = QInputDialog::getText(this, tr("Unlock PDF"), tr("Password:"), QLineEdit::Password, "", &ok);
    }
    if(TDOC->isLocked()){ return; } //Cancelled - still locked
  }

  if(DOC!=0){
    //Clear out the old document first
    delete DOC;
    DOC=0;
  }
  DOC = TDOC; //Save this for later
  qDebug() << "Opening File:" << path;
  this->setWindowTitle(DOC->title());
  if(this->windowTitle().isEmpty()){ this->setWindowTitle(path.section("/",-1)); }

  //Setup the Document
  Poppler::Page *PAGE = DOC->page(0);
  if(PAGE!=0){
    lastdir = path.section("/",0,-2); //save this for later
    Printer->setPageSize( QPageSize(PAGE->pageSize(), QPageSize::Point) ); 
    Printer->setPageMargins(QMarginsF(0,0,0,0), QPageLayout::Point);
    switch(PAGE->orientation()){
	case Poppler::Page::Landscape:
	  Printer->setOrientation(QPrinter::Landscape); break;
	default:
	  Printer->setOrientation(QPrinter::Portrait);
    }
    delete PAGE;
    WIDGET->updatePreview(); //start loading the file preview
  }

}

void MainUI::paintOnWidget(QPrinter *PRINTER){
  if(DOC==0){ return; }
  //this->show();
  //QApplication::processEvents();
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
  //QScreen *scrn = QApplication::screens().first();
  //QSize SDPI(scrn->logicalDotsPerInchX(), scrn->logicalDotsPerInchY());
  QPainter painter(PRINTER);
  //qDebug() << "Set progress bar range:" << firstpage+1 << pages;
  progress->setRange(firstpage+1,pages+1);
  //progress->setValue(firstpage);
  progAct->setVisible(true);
    qDebug() << "Printer DPI:" << DPI;
    //qDebug() << "Screen DPI:" << SDPI;
    for(int i=firstpage; i<pages; i++){
      //qDebug() << "Loading Page:" << i;
      progress->setValue(i+1);
      //qDebug() << " - ProcessEvents";
      QApplication::processEvents();
      //Now paint this page on the printer
      //qDebug() << " - Load Poppler Page";
      if(i != firstpage){ PRINTER->newPage(); } //this is the start of the next page (not needed for first)
      Poppler::Page *PAGE = DOC->page(i);
      if(PAGE!=0){
        painter.drawImage(0,0,PAGE->renderToImage(2*DPI.width(), 2*DPI.height()).scaled(size.width(), size.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );
      }else{
        painter.drawImage(0,0,QImage()); 
      }
      if(PAGE!=0){ delete PAGE; }
      //QApplication::processEvents();
    }
  //qDebug() << "Done Loading Pages";
  progAct->setVisible(false);
}

void MainUI::OpenNewFile(){
  //Prompt for a file
  QString path = QFileDialog::getOpenFileName(this, tr("Open PDF"), lastdir, tr("PDF Documents (*.pdf)"));
  //Now Open it
  if(!path.isEmpty()){ loadFile(path); }
  
}
