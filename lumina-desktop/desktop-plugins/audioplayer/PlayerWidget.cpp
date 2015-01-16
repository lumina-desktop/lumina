//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "PlayerWidget.h"
#include "ui_PlayerWidget.h"

#include <QDir>
#include <QUrl>
#include <QInputDialog>
#include <QFileDialog>
#include <LuminaXDG.h>

PlayerWidget::PlayerWidget(QWidget *parent) : QWidget(parent), ui(new Ui::PlayerWidget()){
  ui->setupUi(this); //load the designer form
  PLAYER = new QMediaPlayer(this);
    PLAYER->setVolume(100);
  PLAYLIST = new QMediaPlaylist(this);
    PLAYLIST->setPlaybackMode(QMediaPlaylist::Sequential);
    PLAYER->setPlaylist(PLAYLIST);
	
  configMenu = new QMenu(this);
    ui->tool_config->setMenu(configMenu);
  addMenu = new QMenu(this);
    ui->tool_add->setMenu(addMenu);
	
  //infoTimer = new QTimer(this);
    //infoTimer->setInterval(30000); //every 30 seconds
	
  updatinglists = false; //start off as false
	
  LoadIcons();
  playerStateChanged(); //update button visibility
  currentSongChanged();
  //Connect all the signals/slots
  //connect(infoTimer, SIGNAL(timeout()), this, SLOT(rotateTrackInfo()) );
  connect(PLAYLIST, SIGNAL(mediaChanged(int, int)), this, SLOT(playlistChanged()) );
  connect(PLAYER, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(playerStateChanged()) );
  connect(PLAYLIST, SIGNAL(currentMediaChanged(const QMediaContent&)), this, SLOT(currentSongChanged()) );
  connect(ui->combo_playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(userListSelectionChanged()) );
  connect(ui->tool_play, SIGNAL(clicked()), this, SLOT(playClicked()) );
  connect(ui->tool_pause, SIGNAL(clicked()), this, SLOT(pauseClicked()) );
  connect(ui->tool_stop, SIGNAL(clicked()), this, SLOT(stopClicked()) );
  connect(ui->tool_next, SIGNAL(clicked()), this, SLOT(nextClicked()) );
  connect(ui->tool_prev, SIGNAL(clicked()), this, SLOT(prevClicked()) );
  
}

PlayerWidget::~PlayerWidget(){
	
}

void PlayerWidget::LoadIcons(){
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

void PlayerWidget::playClicked(){
  PLAYER->play();	
}

void PlayerWidget::pauseClicked(){
  PLAYER->pause();	
}

void PlayerWidget::stopClicked(){
  PLAYER->stop();	
}

void PlayerWidget::nextClicked(){
  PLAYLIST->next();	
}

void PlayerWidget::prevClicked(){
  PLAYLIST->previous();
}

void PlayerWidget::AddFilesToPlaylist(){
  //Prompt the user to select multimedia files
  //Make this use show/processEvents later
  QList<QUrl> files = QFileDialog::getOpenFileUrls(0, tr("Select Multimedia Files"),  QDir::homePath(), "Multimedia Files ("+LXDG::findAVFileExtensions().join(" ")+")");
  if(files.isEmpty()){ return; } //cancelled
  QList<QMediaContent> urls;
  for(int i=0; i<files.length(); i++){
    urls << QMediaContent(files[i]);
  }
  PLAYLIST->addMedia(urls);
  playlistChanged();
}

void PlayerWidget::AddDirToPlaylist(){
  QString dirpath = QFileDialog::getExistingDirectory(0, tr("Select a Multimedia Directory"), QDir::homePath() );
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

void PlayerWidget::AddURLToPlaylist(){
  QString url = QInputDialog::getText(0, tr("Multimedia URL"), tr("Enter a valid URL for a multimedia file or stream"), QLineEdit::Normal);
  if(url.isEmpty()){ return; }
  QUrl newurl(url);
  if(!newurl.isValid()){ return; } //invalid URL
  PLAYLIST->addMedia(newurl);
  playlistChanged();
}

void PlayerWidget::ClearPlaylist(){
  PLAYER->stop();
  PLAYLIST->clear();	
  playlistChanged();
}

void PlayerWidget::ShufflePlaylist(){
  PLAYLIST->shuffle();	
}


void PlayerWidget::userlistSelectionChanged(){ //front-end combobox was changed by the user
  if(updatinglists){ return; }
  PLAYLIST->setCurrentIndex( ui->combo_playlist->currentIndex() );
}

void PlayerWidget::playerStateChanged(){
  switch( PLAYER->state() ){
    case QMediaPlayer::StoppedState:
      ui->tool_stop->setVisible(false);
      ui->tool_play->setVisible(true);
      ui->tool_pause->setVisible(false);
      //infoTimer->stop();
      break;
    case QMediaPlayer::PausedState:
      ui->tool_stop->setVisible(true);
      ui->tool_play->setVisible(true);
      ui->tool_pause->setVisible(false);
      //infoTimer->stop();
      break;
    case QMediaPlayer::PlayingState:
      ui->tool_stop->setVisible(true);
      ui->tool_play->setVisible(false);
      ui->tool_pause->setVisible(true);
      //infoTimer->start();
      break;    
  }
  
}

void PlayerWidget::playlistChanged(){
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

void PlayerWidget::currentSongChanged(){
  if(PLAYLIST->currentIndex() != ui->combo_playlist->currentIndex()){
    updatinglists = true;
    ui->combo_playlist->setCurrentIndex(PLAYLIST->currentIndex());
    updatinglists = false;
  }
  ui->tool_next->setEnabled( PLAYLIST->nextIndex() >= 0 );
  ui->tool_prev->setEnabled( PLAYLIST->previousIndex() >= 0);
  //rotateTrackInfo();
}

/*void PlayerWidget::rotateTrackInfo(){ //on a timer to rotate the visible information about the track
  //NOTE: QMediaPlayer is a type of QMediaObject - so just pull the current info straight out of the player
  ui->label_info->clear();
}*/


AudioPlayerPlugin::AudioPlayerPlugin(QWidget *parent, QString ID) : LDPlugin(parent, ID){
  player = new PlayerWidget(this);
  this->setLayout( new QVBoxLayout() );
    this->layout()->setContentsMargins(0,0,0,0);
    this->layout()->addWidget(player);
	
  this->setInitialSize(300,100);
}

AudioPlayerPlugin::~AudioPlayerPlugin(){
}