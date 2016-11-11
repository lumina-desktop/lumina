//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "PPlayerWidget.h"
#include "ui_PPlayerWidget.h"

#include <QDir>
#include <QUrl>
#include <QInputDialog>
#include <QFileDialog>
#include <LuminaXDG.h>
#include <QDebug>
#include <QDesktopWidget>

PPlayerWidget::PPlayerWidget(QWidget *parent) : QWidget(parent), ui(new Ui::PPlayerWidget()){
  ui->setupUi(this); //load the designer form
  PLAYER = new QMediaPlayer(this);
    PLAYER->setVolume(100);
    PLAYER->setNotifyInterval(1000); //1 second interval (just needs to be a rough estimate)
  PLAYLIST = new QMediaPlaylist(this);
    PLAYLIST->setPlaybackMode(QMediaPlaylist::Sequential);
    PLAYER->setPlaylist(PLAYLIST);
	
  configMenu = new QMenu(this);
    ui->tool_config->setMenu(configMenu);
  addMenu = new QMenu(this);
    ui->tool_add->setMenu(addMenu);
	
  updatinglists = false; //start off as false

  ui->combo_playlist->setContextMenuPolicy(Qt::NoContextMenu);
	
  LoadIcons();
  playerStateChanged(); //update button visibility
  currentSongChanged();
  //Connect all the signals/slots
  //connect(infoTimer, SIGNAL(timeout()), this, SLOT(rotateTrackInfo()) );
  connect(PLAYER, SIGNAL(positionChanged(qint64)),this, SLOT(updateProgress(qint64)) );
  connect(PLAYER, SIGNAL(durationChanged(qint64)), this, SLOT(updateMaxProgress(qint64)) );
  connect(PLAYLIST, SIGNAL(mediaChanged(int, int)), this, SLOT(playlistChanged()) );
  connect(PLAYER, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(playerStateChanged()) );
  connect(PLAYLIST, SIGNAL(currentMediaChanged(const QMediaContent&)), this, SLOT(currentSongChanged()) );
  connect(ui->combo_playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(userlistSelectionChanged()) );
  connect(ui->tool_play, SIGNAL(clicked()), this, SLOT(playClicked()) );
  connect(ui->tool_pause, SIGNAL(clicked()), this, SLOT(pauseClicked()) );
  connect(ui->tool_stop, SIGNAL(clicked()), this, SLOT(stopClicked()) );
  connect(ui->tool_next, SIGNAL(clicked()), this, SLOT(nextClicked()) );
  connect(ui->tool_prev, SIGNAL(clicked()), this, SLOT(prevClicked()) );
  
}

PPlayerWidget::~PPlayerWidget(){
  //qDebug() << "Removing PPlayerWidget";
}

void PPlayerWidget::LoadIcons(){
  ui->tool_stop->setIcon( LXDG::findIcon("media-playback-stop","") );
  ui->tool_play->setIcon( LXDG::findIcon("media-playback-start","") );
  ui->tool_pause->setIcon( LXDG::findIcon("media-playback-pause","") );
  ui->tool_next->setIcon( LXDG::findIcon("media-skip-forward","") );
  ui->tool_prev->setIcon( LXDG::findIcon("media-skip-backward","") );
  ui->tool_add->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_config->setIcon( LXDG::findIcon("configure","") );
  //Now re-assemble the menus as well
  configMenu->clear();
  configMenu->addAction(LXDG::findIcon("media-eject",""), tr("Clear Playlist"), this, SLOT(ClearPlaylist()));
  configMenu->addAction(LXDG::findIcon("roll",""), tr("Shuffle Playlist"), this, SLOT(ShufflePlaylist()));
  addMenu->clear();
  addMenu->addAction(LXDG::findIcon("document-new",""), tr("Add Files"), this, SLOT(AddFilesToPlaylist()));
  addMenu->addAction(LXDG::findIcon("folder-new",""), tr("Add Directory"), this, SLOT(AddDirToPlaylist()));
  addMenu->addAction(LXDG::findIcon("download",""), tr("Add URL"), this, SLOT(AddURLToPlaylist()));
}

void PPlayerWidget::playClicked(){
  PLAYER->play();	
}

void PPlayerWidget::pauseClicked(){
  PLAYER->pause();	
}

void PPlayerWidget::stopClicked(){
  PLAYER->stop();	
}

void PPlayerWidget::nextClicked(){
  PLAYLIST->next();	
}

void PPlayerWidget::prevClicked(){
  PLAYLIST->previous();
}

void PPlayerWidget::AddFilesToPlaylist(){
  //Prompt the user to select multimedia files
  QFileDialog dlg(0, Qt::Dialog | Qt::WindowStaysOnTopHint );
      dlg.setFileMode(QFileDialog::ExistingFiles);
      dlg.setAcceptMode(QFileDialog::AcceptOpen);
      dlg.setNameFilter( tr("Multimedia Files")+" ("+LXDG::findAVFileExtensions().join(" ")+")");
      dlg.setWindowTitle(tr("Select Multimedia Files"));
      dlg.setWindowIcon( LXDG::findIcon("file-open","") );
      dlg.setDirectory(QDir::homePath()); //start in the home directory
      //ensure it is centered on the current screen
      QPoint center = QApplication::desktop()->screenGeometry(this).center();
      dlg.move( center.x()-(dlg.width()/2), center.y()-(dlg.height()/2) );
  dlg.show();
  while( dlg.isVisible() ){
    QApplication::processEvents();
  }
  QList<QUrl> files = dlg.selectedUrls();
  if(files.isEmpty()  || dlg.result()!=QDialog::Accepted){ return; } //cancelled
  //Make this use show/processEvents later
  //QList<QUrl> files = QFileDialog::getOpenFileUrls(0, tr("Select Multimedia Files"),  QDir::homePath(), "Multimedia Files ("+LXDG::findAVFileExtensions().join(" ")+")");
  QList<QMediaContent> urls;
  for(int i=0; i<files.length(); i++){
    urls << QMediaContent(files[i]);
  }
  PLAYLIST->addMedia(urls);
  playlistChanged();
}

void PPlayerWidget::AddDirToPlaylist(){
  QFileDialog dlg(0, Qt::Dialog | Qt::WindowStaysOnTopHint );
      dlg.setFileMode(QFileDialog::Directory);
      dlg.setOption(QFileDialog::ShowDirsOnly, true);
      dlg.setAcceptMode(QFileDialog::AcceptOpen);
      dlg.setWindowTitle(tr("Select Multimedia Directory"));
      dlg.setWindowIcon( LXDG::findIcon("folder-open","") );
      dlg.setDirectory(QDir::homePath()); //start in the home directory
      //ensure it is centered on the current screen
      QPoint center = QApplication::desktop()->screenGeometry(this).center();
      dlg.move( center.x()-(dlg.width()/2), center.y()-(dlg.height()/2) );
  dlg.show();
  while( dlg.isVisible() ){
    QApplication::processEvents();
  }
  if(dlg.result() != QDialog::Accepted){ return; } //cancelled
  QStringList sel = dlg.selectedFiles();
  if(sel.isEmpty()){ return; } //cancelled
  QString dirpath = sel.first(); //QFileDialog::getExistingDirectory(0, tr("Select a Multimedia Directory"), QDir::homePath() );
  if(dirpath.isEmpty()){ return; } //cancelled
  QDir dir(dirpath);
  QFileInfoList files = dir.entryInfoList(LXDG::findAVFileExtensions(), QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
  if(files.isEmpty()){ return; } //nothing in this directory
  QList<QMediaContent> urls;
  for(int i=0; i<files.length(); i++){
    urls << QMediaContent(QUrl::fromLocalFile(files[i].absoluteFilePath()) );
  }
  PLAYLIST->addMedia(urls);
  playlistChanged();
}

void PPlayerWidget::AddURLToPlaylist(){
  QInputDialog dlg(0, Qt::Dialog | Qt::WindowStaysOnTopHint );
      dlg.setInputMode(QInputDialog::TextInput);
      dlg.setLabelText(tr("Enter a valid URL for a multimedia file or stream:"));
      dlg.setTextEchoMode(QLineEdit::Normal);
      dlg.setWindowTitle(tr("Multimedia URL"));
      dlg.setWindowIcon( LXDG::findIcon("download","") );
      //ensure it is centered on the current screen
      QPoint center = QApplication::desktop()->screenGeometry(this).center();
      dlg.move( center.x()-(dlg.width()/2), center.y()-(dlg.height()/2) );
  dlg.show();
  while( dlg.isVisible() ){
    QApplication::processEvents();
  }
  QString url = dlg.textValue();
  if(url.isEmpty() || dlg.result()!=QDialog::Accepted){ return; } //cancelled
	
  //QString url = QInputDialog::getText(0, tr("Multimedia URL"), tr("Enter a valid URL for a multimedia file or stream"), QLineEdit::Normal);
  //if(url.isEmpty()){ return; }
  QUrl newurl(url);
  if(!newurl.isValid()){ return; } //invalid URL
  PLAYLIST->addMedia(newurl);
  playlistChanged();
}

void PPlayerWidget::ClearPlaylist(){
  PLAYER->stop();
  PLAYLIST->clear();	
  playlistChanged();
}

void PPlayerWidget::ShufflePlaylist(){
  PLAYLIST->shuffle();	
}


void PPlayerWidget::userlistSelectionChanged(){ //front-end combobox was changed by the user
  if(updatinglists){ return; }
  PLAYLIST->setCurrentIndex( ui->combo_playlist->currentIndex() );
}

void PPlayerWidget::playerStateChanged(){
  switch( PLAYER->state() ){
    case QMediaPlayer::StoppedState:
      ui->tool_stop->setVisible(false);
      ui->tool_play->setVisible(true);
      ui->tool_pause->setVisible(false);
      ui->progressBar->setVisible(false);
      break;
    case QMediaPlayer::PausedState:
      ui->tool_stop->setVisible(true);
      ui->tool_play->setVisible(true);
      ui->tool_pause->setVisible(false);
      ui->progressBar->setVisible(true);
      break;
    case QMediaPlayer::PlayingState:
      ui->tool_stop->setVisible(true);
      ui->tool_play->setVisible(false);
      ui->tool_pause->setVisible(true);
      ui->progressBar->setVisible(true);
      break;    
  }
  
}

void PPlayerWidget::playlistChanged(){
  updatinglists = true;
  ui->combo_playlist->clear();
  for(int i=0; i<PLAYLIST->mediaCount(); i++){
    QUrl url = PLAYLIST->media(i).canonicalUrl();
    if(url.isLocalFile()){
      ui->combo_playlist->addItem(LXDG::findMimeIcon(url.fileName().section(".",-1)), url.fileName() );	
    }else{
      ui->combo_playlist->addItem(LXDG::findIcon("download",""), url.toString() );
    }
  }
  if(PLAYLIST->currentIndex()<0 && PLAYLIST->mediaCount()>0){ PLAYLIST->setCurrentIndex(0); }
  ui->combo_playlist->setCurrentIndex(PLAYLIST->currentIndex());
  
  updatinglists = false;
}

void PPlayerWidget::currentSongChanged(){
  if(PLAYLIST->currentIndex() != ui->combo_playlist->currentIndex()){
    updatinglists = true;
    ui->combo_playlist->setCurrentIndex(PLAYLIST->currentIndex());
    updatinglists = false;
  }
  ui->tool_next->setEnabled( PLAYLIST->nextIndex() >= 0 );
  ui->tool_prev->setEnabled( PLAYLIST->previousIndex() >= 0);
  ui->label_num->setText( QString::number( PLAYLIST->currentIndex()+1)+"/"+QString::number(PLAYLIST->mediaCount()) );
  ui->progressBar->setRange(0, PLAYER->duration() );
  ui->progressBar->setValue(0);
}

void PPlayerWidget::updateProgress(qint64 val){
  //qDebug() << "Update Progress Bar:" << val;
  ui->progressBar->setValue(val);
}

void PPlayerWidget::updateMaxProgress(qint64 val){
  ui->progressBar->setRange(0,val);	
}
