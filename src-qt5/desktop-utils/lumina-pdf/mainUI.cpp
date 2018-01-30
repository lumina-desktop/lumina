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
#include <QTimer>
#include <iostream>

#include <QtConcurrent>

#include <LuminaXDG.h>
#include "PrintWidget.h"

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI()){
  ui->setupUi(this);
  this->setWindowTitle(tr("Lumina PDF Viewer"));
  this->setWindowIcon( LXDG::findIcon("application-pdf","unknown"));
  presentationLabel = 0;
  CurrentPage = 1;
  lastdir = QDir::homePath();
  //Create the interface widgets
  WIDGET = new PrintWidget(this->centralWidget());
  WIDGET->setVisible(false);
  WIDGET->setContextMenuPolicy(Qt::CustomContextMenu);
  WIDGET->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  clockTimer = new QTimer(this);
    clockTimer->setInterval(1000); //1-second updates to clock
    connect(clockTimer, SIGNAL(timeout()), this, SLOT(updateClock()) );
  //frame_presenter = new QFrame(this);
  label_clock = new QLabel(this);
    label_clock->setAlignment(Qt::AlignCenter );
    label_clock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label_clock->setStyleSheet("QLabel{color: palette(highlight-text); background-color: palette(highlight); border-radius: 5px; }");

  label_page = new QLabel(this);
    label_page->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    label_page->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
  //Context Menu
  contextMenu = new QMenu(this);
    connect(contextMenu, SIGNAL(aboutToShow()), this, SLOT(updateContextMenu()));
  //Now put the widgets into the UI
  ui->bookmarksFrame->setParent(WIDGET);
  ui->findGroup->setParent(WIDGET);
  qDebug() << "Setting central widget";
  this->centralWidget()->layout()->replaceWidget(ui->label_replaceme, WIDGET); //setCentralWidget(WIDGET);
  ui->label_replaceme->setVisible(false);
  WIDGET->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(WIDGET, SIGNAL(customContextMenuRequested(const QPoint&)),this, SLOT(showContextMenu(const QPoint&)) );
  connect(WIDGET, SIGNAL(currentPageChanged()), this, SLOT(updatePageNumber()) );
  DOC = 0;
  connect(this, SIGNAL(PageLoaded(int)), this, SLOT(slotPageLoaded(int)) );
  connect(this, SIGNAL(sendDocument(Poppler::Document*)), WIDGET, SLOT(receiveDocument(Poppler::Document*)));

  PrintDLG = new QPrintDialog(this);
  connect(PrintDLG, SIGNAL(accepted(QPrinter*)), this, SLOT(paintToPrinter(QPrinter*)) );
  //connect(ui->menuStart_Presentation, SIGNAL(triggered(QAction*)), this, SLOT(slotStartPresentation(QAction*)) );

  //Create the other interface widgets
  progress = new QProgressBar(this);
    progress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    progress->setFormat("%v/%m (%p%)"); // [current]/[total]
  progAct = ui->toolBar->addWidget(progress);
    progAct->setVisible(false);
  clockAct = ui->toolBar->addWidget(label_clock);
    clockAct->setVisible(false);
  pageAct = ui->toolBar->addWidget(label_page);
    pageAct->setVisible(false);

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

  //qDebug() << "Starting connections";

  //Connect up the buttons
  connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(close()) );
  connect(ui->actionPrint, SIGNAL(triggered()), PrintDLG, SLOT(open()) );
  connect(ui->actionFit_Width, SIGNAL(triggered()), WIDGET, SLOT(fitToWidth()) );
  connect(ui->actionFit_Page, SIGNAL(triggered()), WIDGET, SLOT(fitView()) );
  connect(ui->actionOpen_PDF, SIGNAL(triggered()), this, SLOT(OpenNewFile()) );
  connect(ui->actionSingle_Page, SIGNAL(triggered()), WIDGET, SLOT(setSinglePageViewMode()) );
  connect(ui->actionDual_Pages, SIGNAL(triggered()), WIDGET, SLOT(setFacingPagesViewMode()) );
  connect(ui->actionAll_Pages, SIGNAL(triggered()), WIDGET, SLOT(setAllPagesViewMode()) );
  //connect(ui->actionScroll_Mode,  &QAction::triggered, this, [&] { this->setScroll(true); });
  //connect(ui->actionSelect_Mode, &QAction::triggered, this, [&] { this->setScroll(false); });
  connect(ui->actionZoom_In,  &QAction::triggered, WIDGET, [&] { WIDGET->zoomIn(1.2);  });
  connect(ui->actionZoom_Out, &QAction::triggered, WIDGET, [&] { WIDGET->zoomOut(1.2); });
  connect(ui->actionRotate_Counterclockwise, &QAction::triggered, this, [&] { WIDGET->setDegrees(-90); });
  connect(ui->actionRotate_Clockwise, &QAction::triggered, this, [&] { WIDGET->setDegrees(90); });
  connect(ui->actionZoom_In_2,  &QAction::triggered, WIDGET, [&] { WIDGET->zoomIn(1.2);  });
  connect(ui->actionZoom_Out_2, &QAction::triggered, WIDGET, [&] { WIDGET->zoomOut(1.2); });
  connect(ui->actionFirst_Page, SIGNAL(triggered()), this, SLOT(firstPage()) );
  connect(ui->actionPrevious_Page, SIGNAL(triggered()), this, SLOT(prevPage()) );
  connect(ui->actionNext_Page, SIGNAL(triggered()), this, SLOT(nextPage()) );
  connect(ui->actionLast_Page, SIGNAL(triggered()), this, SLOT(lastPage()) );
  connect(ui->actionProperties, SIGNAL(triggered()), this, SLOT(showInformation()));
  connect(ui->actionFind, SIGNAL(triggered()), this, SLOT(enableFind()));
  connect(ui->actionFind_Next,  &QAction::triggered, this,
    [&] { find(ui->textEdit->text(), true); });
  connect(ui->actionFind_Previous,  &QAction::triggered, this,
    [&] { find(ui->textEdit->text(), false); });
  connect(ui->findNextB,  &QPushButton::clicked, this,
    [&] { find(ui->textEdit->text(), true); });
  connect(ui->findPrevB,  &QPushButton::clicked, this,
    [&] { find(ui->textEdit->text(), false); });
  connect(ui->matchCase, &QPushButton::clicked, this,
    [&] (bool value) { this->matchCase = value; });
  connect(ui->closeFind, &QPushButton::clicked, this,
    [&] { ui->findGroup->setVisible(false); this->setFocus(); });
  connect(ui->actionClearHighlights,  &QAction::triggered, WIDGET,
    [&] { WIDGET->updatePreview(); });
  connect(ui->actionBookmarks, SIGNAL(triggered()), this, SLOT(showBookmarks()));

  //qDebug() << "Finished connctions";

  //int curP = WIDGET->currentPage()-1; //currentPage reports pages starting at 1
  //int lastP = numPages-1;
  ui->actionFirst_Page->setText(tr("First Page"));
  ui->actionPrevious_Page->setText(tr("Previous Page"));
  ui->actionNext_Page->setText(tr("Next Page"));
  ui->actionLast_Page->setText(tr("Last Page"));
  /*ui->actionFirst_Page->setEnabled(curP!=0);
  ui->actionPrevious_Page->setEnabled(curP>0);
  ui->actionNext_Page->setEnabled(curP<lastP);
  ui->actionLast_Page->setEnabled(curP!=lastP);*/

  ui->actionStart_Here->setText(tr("Start Presentation (current slide)"));
  connect(ui->actionStart_Here, SIGNAL(triggered()), this, SLOT(startPresentationHere()) );
  ui->actionStart_Begin->setText(tr("Start Presentation (at beginning)"));
  connect(ui->actionStart_Begin, SIGNAL(triggered()), this, SLOT(startPresentationBeginning()) );
  ui->actionStop_Presentation->setText(tr("Stop Presentation"));
  connect(ui->actionStop_Presentation, SIGNAL(triggered()), this, SLOT(closePresentation()) );

  //Setup all the icons
  ui->actionPrint->setIcon( LXDG::findIcon("document-print",""));
  ui->actionClose->setIcon( LXDG::findIcon("window-close",""));
  ui->actionFit_Width->setIcon(LXDG::findIcon("transform-scale",""));
  ui->actionFit_Page->setIcon(LXDG::findIcon("zoom-fit-best",""));
  ui->actionOpen_PDF->setIcon(LXDG::findIcon("document-open",""));
  ui->actionSingle_Page->setIcon(LXDG::findIcon("format-view-agenda",""));
  ui->actionDual_Pages->setIcon(LXDG::findIcon("format-view-grid-small",""));
  ui->actionAll_Pages->setIcon(LXDG::findIcon("format-view-grid-large",""));
  ui->actionScroll_Mode->setIcon(LXDG::findIcon("cursor-pointer",""));
  ui->actionSelect_Mode->setIcon(LXDG::findIcon("cursor-text",""));
  ui->actionZoom_In->setIcon(LXDG::findIcon("zoom-in",""));
  ui->actionZoom_Out->setIcon(LXDG::findIcon("zoom-out",""));
  ui->actionZoom_In_2->setIcon(LXDG::findIcon("zoom-in",""));
  ui->actionZoom_Out_2->setIcon(LXDG::findIcon("zoom-out",""));
  ui->actionRotate_Counterclockwise->setIcon(LXDG::findIcon("object-rotate-left",""));
  ui->actionRotate_Clockwise->setIcon(LXDG::findIcon("object-rotate-right",""));
  ui->actionFirst_Page->setIcon(LXDG::findIcon("go-first",""));
  ui->actionPrevious_Page->setIcon(LXDG::findIcon("go-previous",""));
  ui->actionNext_Page->setIcon(LXDG::findIcon("go-next",""));
  ui->actionLast_Page->setIcon(LXDG::findIcon("go-last",""));
  ui->actionStart_Here->setIcon(LXDG::findIcon("media-playback-start-circled",""));
  ui->actionStart_Begin->setIcon(LXDG::findIcon("presentation-play",""));
  ui->actionStop_Presentation->setIcon(LXDG::findIcon("media-playback-stop-circled",""));
  ui->actionBookmarks->setIcon(LXDG::findIcon("bookmark-new",""));
  ui->actionFind->setIcon(LXDG::findIcon("edit-find",""));
  ui->actionFind_Next->setIcon(LXDG::findIcon("edit-find-next",""));
  ui->actionFind_Previous->setIcon(LXDG::findIcon("edit-find-prev",""));
  ui->actionProperties->setIcon(LXDG::findIcon("dialog-information",""));
  ui->actionSettings->setIcon(LXDG::findIcon("document-properties",""));
  ui->findNextB->setIcon(LXDG::findIcon("go-down-search"));
  ui->findPrevB->setIcon(LXDG::findIcon("go-up-search"));
  ui->matchCase->setIcon(LXDG::findIcon("format-text-italic"));
  ui->closeFind->setIcon(LXDG::findIcon("dialog-close"));

  //qDebug() << "Finished setting icons";

  //Now set the default state of the menu's and actions
  ui->actionStop_Presentation->setEnabled(false);
  ui->actionStart_Here->setEnabled(false);
  ui->actionStart_Begin->setEnabled(false);

  ui->findGroup->setVisible(false);
  ui->bookmarksFrame->setVisible(false);
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
  //qpdf.processFile(path.toLatin1().data(), pass.toLatin1().data());

  if(DOC!=0){
    //Clear out the old document first
    delete DOC;
    DOC=0;
  }
  //loadingHash.clear(); //clear out this hash
  numPages = -1;
  DOC = TDOC; //Save this for later
  qDebug() << "Opening File:" << path;
  this->setWindowTitle(TDOC->title());
  if(this->windowTitle().isEmpty()){ this->setWindowTitle(path.section("/",-1)); }

  //Setup the Document
  //QVector<QPDFObjectHandle> pages = (QVector<QPDFObjectHandle>)pdf.getAllPages();
  //QPDFObjectHandle page = pages.at(0);
  Poppler::Page *PAGE = DOC->page(0);
  if(PAGE!=0){
    lastdir = path.section("/",0,-2); //save this for later
    /*switch(PAGE->orientation()){
      case Poppler::Page::Landscape:
        WIDGET->setOrientation(QPageLayout::Landscape); break;
      default:
        WIDGET->setOrientation(QPageLayout::Portrait);
    }*/
    delete PAGE;
    qDebug() << " - Document Setup : start loading pages now";
    startLoadingPages();
    //QTimer::singleShot(10, WIDGET, SLOT(updatePreview())); //start loading the file preview
  }

}

void MainUI::loadPage(int num, Poppler::Document *doc, MainUI *obj, QSize dpi, QSizeF page){
  //qDebug() << " - Render Page:" << num;
  Poppler::Page *PAGE = doc->page(num);
  if(PAGE!=0){
    QImage raw = PAGE->renderToImage(dpi.width(),dpi.height()); //make the raw image a bit larger than the end result
    //qDebug() << " - Raw Image Size:" << raw.size();
    loadingHash.insert(num, raw.scaled(page.width(), page.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );
    raw = QImage(); //clear it
    /*
    QList<Annotation*> anno = PAGE->annotations(Annotations::AText );
    QStringList annoS;
    for(int i=0; i<anno.length(); i++){
      annoS << static_cast<TextAnnotation*>(anno[i])->???
    }
    annotateHash.insert(num, PAGE->
    */
  }else{
    loadingHash.insert(num, QImage());
  }
  if(PAGE!=0){ delete PAGE; }
  obj->emit PageLoaded(num);
}

QScreen* MainUI::getScreen(bool current, bool &cancelled){
  //Note: the "cancelled" boolian is actually an output - not an input
  QList<QScreen*> screens = QApplication::screens();
  cancelled = false;
  if(screens.length() ==1){ return screens[0]; } //only one option
  //Multiple screens available - figure it out
  if(current){
    //Just return the screen the window is currently on
    for(int i=0; i<screens.length(); i++){
      if(screens[i]->geometry().contains( this->mapToGlobal(this->pos()) )){
        return screens[i];
      }
    }
    //If it gets this far, there was an error and it should just return the primary screen
    return QApplication::primaryScreen();
  }else{
    //Ask the user to select a screen (for presentations, etc..)
    QStringList names;
    for(int i=0; i<screens.length(); i++){
      QString screensize = QString::number(screens[i]->size().width())+"x"+QString::number(screens[i]->size().height());
       names << QString(tr("%1 (%2)")).arg(screens[i]->name(), screensize);
    }
    bool ok = false;
    QString sel = QInputDialog::getItem(this, tr("Select Screen"), tr("Screen:"), names, 0, false, &ok);
    cancelled = !ok;
    if(!ok){ return screens[0]; } //cancelled - just return the first one
    int index = names.indexOf(sel);
    if(index < 0){ return screens[0]; } //error - should never happen though
    return screens[index]; //return the selected screen
  }
}

void MainUI::startPresentation(bool atStart){
  if(DOC==0){ return; } //just in case
  bool cancelled = false;
  QScreen *screen = getScreen(false, cancelled); //let the user select which screen to use (if multiples)
  if(cancelled){ return;}
  int page = 1;
  if(!atStart){ page = WIDGET->currentPage(); }
  //PDPI = QSize(SCALEFACTOR*screen->physicalDotsPerInchX(), SCALEFACTOR*screen->physicalDotsPerInchY());
  //Now create the full-screen window on the selected screen
  if(presentationLabel == 0){
    //Create the label and any special flags for it
    presentationLabel = new PresentationLabel();
      presentationLabel->setStyleSheet("background-color: black;");
      presentationLabel->setAlignment(Qt::AlignCenter);
      presentationLabel->setContextMenuPolicy(Qt::CustomContextMenu);
      connect(presentationLabel, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)) );
      connect(presentationLabel, SIGNAL(nextSlide()), this, SLOT(nextPage()) );
  }
  //Now put the label in the proper location
  presentationLabel->setGeometry(screen->geometry());
  presentationLabel->showFullScreen();

  ui->actionStop_Presentation->setEnabled(true);
  ui->actionStart_Here->setEnabled(false);
  ui->actionStart_Begin->setEnabled(false);
  updateClock();
  updatePageNumber();
  clockAct->setVisible(true);
  clockTimer->start();
  QApplication::processEvents();
  //Now start at the proper page
  ShowPage(page);
  this->grabKeyboard(); //Grab any keyboard events - even from the presentation window
}

void MainUI::ShowPage(int page){
  //Check for valid document/page
  //qDebug() << "Load Page:" << page << "/" << numPages << "Index:" << page;
  if(page<0 || page > numPages+1 || (page==numPages && CurrentPage==page) ){
    endPresentation();
    return; //invalid - no document loaded or invalid page specified
  }
  WIDGET->setCurrentPage(page); //page numbers start at 1 for this widget
  //Stop here if no presentation currently running

  if(presentationLabel == 0 || !presentationLabel->isVisible()){ return; }
  //qDebug() << "Show Page:" << page << "/" << numPages;
  CurrentPage = page;
  QImage PAGEIMAGE;
  if(page<numPages+1){ PAGEIMAGE = loadingHash[page-1]; }

  //Now scale the image according to the user-designations and show it
  if(!PAGEIMAGE.isNull()){
    QPixmap pix;
    pix.convertFromImage( PAGEIMAGE.scaled( presentationLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
    presentationLabel->setPixmap(pix);
    presentationLabel->show(); //always make sure it was not hidden
  }else{
    //Blank page (useful so there is one blank page after the last slide before stopping the presentation)
    presentationLabel->setPixmap(QPixmap());
  }
}

void MainUI::endPresentation(){
  if(presentationLabel==0 || !presentationLabel->isVisible()){ return; } //not in presentation mode
  presentationLabel->hide(); //just hide this (no need to re-create the label for future presentations)
  ui->actionStop_Presentation->setEnabled(false);
  ui->actionStart_Here->setEnabled(true);
  ui->actionStart_Begin->setEnabled(true);
  clockTimer->stop();
  clockAct->setVisible(false);
  this->releaseKeyboard();
  updatePageNumber();
}

void MainUI::startLoadingPages(){
  if(numPages>0){ return; } //currently loaded[ing]
  //qDebug() << " - Start Loading Pages";
  loadingHash.clear();
  numPages = DOC->numPages();
  //qDebug() << "numPages:" << numPages;
  progress->setRange(0,numPages);
  progress->setValue(0);
  progAct->setVisible(true);
  pageAct->setVisible(false);
  //PERFORMANCE NOTES:
  // Using Poppler to scale the image (adjust dpi value) helps a bit but you take a larger CPU loading hit (and still quite a lot of pixelization)
  // Using Qt to scale the image (adjust page value) smooths out the image quite a bit without a lot of performance loss (but cannot scale up without pixelization)
  // The best approach seams to be to increase the DPI a bit, but match that with the same scaling on the page size (smoothing)

  double scalefactor = 2.5;
  QSizeF pageSize = DOC->page(0)->pageSizeF()*scalefactor;
  //QSize DPI(loadingHash[0]->resolution(),loadingHash[0]->resolution());
  //QSize DPI(76,76);
  //DPI = DPI*(scalefactor+1); //need this a bit higher than the page scaling
  QSize DPI(300,300); //print-quality (some printers even go to 600 DPI nowdays)

  /*qDebug() << "Screen Resolutions:";
  QList<QScreen*> screens = QApplication::screens();
  for(int i=0; i<screens.length(); i++){
    qDebug() << screens[i]->name() << screens[i]->logicalDotsPerInchX() << screens[i]->logicalDotsPerInchY();
  }*/
  //qDebug() << "Poppler pageSize: " << pageSize;
  for(int i=0; i<numPages; i++){
    //qDebug() << " - Kickoff page load:" << i;
    QtConcurrent::run(this, &MainUI::loadPage, i, DOC, this, DPI, pageSize);
  }
}

void MainUI::slotPageLoaded(int page){
  Q_UNUSED(page);
  //qDebug() << "Page Loaded:" << page;
  int finished = loadingHash.keys().length();
  //qDebug() << " - finished:" << finished;
  if(finished == numPages){
    progAct->setVisible(false);
    WIDGET->setPictures(&loadingHash);
    emit sendDocument(DOC);
    ui->actionStop_Presentation->setEnabled(false);
    ui->actionStart_Here->setEnabled(true);
    ui->actionStart_Begin->setEnabled(true);
    pageAct->setVisible(true);
  }else{
    progress->setValue(finished);
  }
}

void MainUI::paintToPrinter(QPrinter *PRINTER){
  if(loadingHash.keys().length() != numPages){ return; }

  int pages = DOC->numPages();
  int firstpage = 0;
  int copies = PRINTER->copyCount();
  bool collate = PRINTER->collateCopies();
  qDebug() << "PRINTER DPI:" << PRINTER->resolution() << PRINTER->supportedResolutions();
  return;
  if(PRINTER->resolution() < 300){
    //Try to get 300 DPI resolution at least
    PRINTER->setResolution(300);
    qDebug() << "Trying to change print resolution to 300 minimum";
    qDebug() << "  -- Resolutions listed as supported:" << PRINTER->supportedResolutions();
  }
  //bool duplex = (PRINTER->duplex()!=QPrinter::DuplexNone);
  //Determine the first page that needs to be printed, and the range
  if((PRINTER->fromPage() != PRINTER->toPage() || PRINTER->fromPage()!=0 ) && PRINTER->printRange()==QPrinter::PageRange ){
    firstpage = PRINTER->fromPage() - 1;
    pages = PRINTER->toPage();
  }
  qDebug() << "Start Printing PDF: Pages" << PRINTER->fromPage() <<" to "<< PRINTER->toPage()<< " Copies:" << copies << "  collate:" << collate;
  QList<int> pageCount;
  //Assemble the page order/count based on printer settings
  for(int i=firstpage; i<pages; i++){
    //Make sure even/odd pages are not selected as desired
    //Qt 5.7.1 does not seem to have even/odd page selections - 8/11/2017
    pageCount << i; //add this page to the list
    for(int c=1; c<copies && !collate; c++){ pageCount << i; } //add any copies of this page as needed
  }
  //qDebug() << "Got Page Range:" << pageCount;
  if(PRINTER->pageOrder()==QPrinter::LastPageFirst){
    //Need to reverse the order of the list
    QList<int> tmp = pageCount;
    pageCount.clear();
    for(int i=tmp.length()-1; i>=0; i--){ pageCount << tmp[i]; }
    //qDebug() << " - reversed:" << pageCount;
  }
  if(collate && copies>0){
    QList<int> orig = pageCount; //original array of pages
    for(int c=1; c<copies; c++){
      pageCount << orig; //add a new copy of the entire page range
    }
  }
  //qDebug() << "Final Page Range:" << pageCount;
  //Generate the sizing information for the printer
  QSize sz(PRINTER->pageRect().width(), PRINTER->pageRect().height());
  bool landscape = PRINTER->orientation()==QPrinter::Landscape;
  if(landscape){ sz = QSize(sz.height(), sz.width() ); } //flip the size dimensions as needed
  //Now send out the pages in the right order/format
  QPainter painter(PRINTER);
    //Ensure all the antialiasing/smoothing options are turned on
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

  QTransform transF;
  transF.rotate(90);
  //Show the progress bar
  progAct->setVisible(true);
  progress->setRange(0, pageCount.length()-1);
  for(int i=0; i<pageCount.length(); i++){
    if(i!=0){ PRINTER->newPage(); }
    progress->setValue(i);
    QApplication::processEvents();
    QImage img = loadingHash[pageCount[i]].scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    //Now draw the image
    painter.drawImage(0,0,img);
  }
  progAct->setVisible(false);
}

void MainUI::OpenNewFile(){
  //Prompt for a file
  QString path = QFileDialog::getOpenFileName(this, tr("Open PDF"), lastdir, tr("PDF Documents (*.pdf)"));
  //Now Open it
  if(!path.isEmpty()){ loadFile(path); }
}

void MainUI::updateClock(){
  label_clock->setText( QDateTime::currentDateTime().toString("<b>hh:mm:ss</b>") );
}

void MainUI::updatePageNumber(){
  QString text;
  if(presentationLabel==0 || !presentationLabel->isVisible()){ text = tr("Page %1 of %2"); }
  else{ text = "%1/%2"; }
  label_page->setText( text.arg( QString::number(WIDGET->currentPage()), QString::number(numPages) ));
}

/*void MainUI::setScroll(bool tog) {
  if(tog) {
    QApplication::setOverrideCursor(Qt::OpenHandCursor);
  }else{
    QApplication::setOverrideCursor(Qt::IBeamCursor);
  }
}*/

void MainUI::updateContextMenu(){
  contextMenu->clear();
  contextMenu->addSection( QString(tr("Page %1 of %2")).arg(QString::number(WIDGET->currentPage()),
    QString::number(numPages) ) );
  contextMenu->addAction(ui->actionPrevious_Page);
  contextMenu->addAction(ui->actionNext_Page);
  contextMenu->addSeparator();
  contextMenu->addAction(ui->actionFirst_Page);
  contextMenu->addAction(ui->actionLast_Page);
  contextMenu->addSeparator();
  if(presentationLabel==0 || !presentationLabel->isVisible()){
    contextMenu->addAction(ui->actionStart_Begin);
    contextMenu->addAction(ui->actionStart_Here);
  }else{
    contextMenu->addAction(ui->actionStop_Presentation);
  }
}

void MainUI::keyPressEvent(QKeyEvent *event){
  //See if this is one of the special hotkeys and act appropriately
  bool inPresentation = (presentationLabel!=0);
  if( event->key()==Qt::Key_Escape || event->key()==Qt::Key_Backspace){
    if(inPresentation){ endPresentation(); }
  }else if(event->key()==Qt::Key_Right || event->key()==Qt::Key_Space ||
  event->key()==Qt::Key_PageDown){
    nextPage();
  }else if(event->key()==Qt::Key_Left || event->key()==Qt::Key_PageUp){
    prevPage();
  }else if(event->key()==Qt::Key_Home){
    firstPage();
  }else if(event->key()==Qt::Key_End){
    lastPage();
  }else if(event->key()==Qt::Key_F11){
    if(inPresentation){ endPresentation(); }
    else{ startPresentationHere(); }
  }else if(event->key() == Qt::Key_Up) {
    //Scroll the widget up
  }else if(event->key() == Qt::Key_Down) {
    //Scroll the widget down
    /*qDebug() << "Send Wheel Event";
    QWheelEvent wEvent( WIDGET->mapFromGlobal(QCursor::pos()), QCursor::pos(),QPoint(0,0), QPoint(0,30), 0, Qt::Vertical, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(WIDGET, &wEvent);*/
  }else{
    QMainWindow::keyPressEvent(event);
  }
}

void MainUI::wheelEvent(QWheelEvent *event) {
  //Scroll the window according to the mouse wheel
  QMainWindow::wheelEvent(event);
}

void MainUI::showInformation() {
  PROPDIALOG = new PropDialog(DOC);
  PROPDIALOG->show();
}

void MainUI::find(QString text, bool forward) {
  if(!text.isEmpty()) {
    qDebug() << "Finding Text";
    bool newText = results.empty();
    bool research = false;
    if(!newText)
      research = !results.keys()[0]->text().contains(text);
    //Clear results if the user gives a new search string
    if(research)
      results.clear();

    if(research or newText) {
      for(int i = 0; i < numPages; i++) {
        QList<Poppler::TextBox*> textList = DOC->page(i)->textList();
        for(int j = 0; j < textList.size(); j++) {
          if(textList[j]->text().contains(text, (matchCase)
              ? Qt::CaseSensitive : Qt::CaseInsensitive)) {
            results.insert(textList[j], i);
          }
        }
      }
      currentHighlight = (forward) ? -1 : results.size();
    }

    qDebug() << "Jumping to next result";
    if(!results.empty()) {
      //Jump to the location of the next or previous textbox and highlight
      if(forward) {
        currentHighlight = (currentHighlight + 1) % results.size();
      }else{
        currentHighlight--;
        //Ensure currentHighlight will be between 0 and results.size() - 1
        if(currentHighlight < 0)
          currentHighlight = results.size() - 1;
      }

      qDebug() << "Jump to location: " << currentHighlight;

      Poppler::TextBox *currentText = results.keys()[currentHighlight];
      WIDGET->setCurrentPage(results.value(currentText));
      WIDGET->highlightText(currentHighlight, currentText->boundingBox());

      QTimer::singleShot(10, WIDGET, SLOT(updatePreview()));
    }else{
      //Print "No results found"
    }
  }
}

void MainUI::enableFind() {
  if(ui->findGroup->isVisible()) {
    qDebug() << "Disabling Find";
    ui->findGroup->setVisible(false);
    WIDGET->setGeometry(QRect(WIDGET->pos(),
      QSize(WIDGET->width(), WIDGET->height()+ui->findGroup->height())));
    QTimer::singleShot(0, WIDGET, SLOT(updatePreview()));
    this->setFocus();
  }else{
    qDebug() << "Enabling Find";
    ui->findGroup->setGeometry(QRect(QPoint(0, WIDGET->height()-ui->findGroup->height()),
      QSize(WIDGET->width()-12, ui->findGroup->height())));
    ui->findGroup->setVisible(true);
    WIDGET->setGeometry(QRect(WIDGET->pos(),
      QSize(WIDGET->width(), WIDGET->height()-ui->findGroup->height())));

    QTimer::singleShot(0, WIDGET, SLOT(updatePreview()));
    ui->findGroup->setFocus();
  }
}

void MainUI::showBookmarks() {
  ui->bookmarksFrame->setVisible(true);
}

void MainUI::resizeEvent(QResizeEvent *event) {
  if(ui->findGroup->isVisible()) {
    ui->findGroup->setGeometry(QRect(QPoint(0, WIDGET->height()-ui->findGroup->height()),
      QSize(WIDGET->width()-10, ui->findGroup->height())));
  }
  QMainWindow::resizeEvent(event);
}
