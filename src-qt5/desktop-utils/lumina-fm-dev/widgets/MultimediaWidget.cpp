//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MultimediaWidget.h"
#include "ui_MultimediaWidget.h"

#include <QTimer>

MultimediaWidget::MultimediaWidget(QWidget *parent) : QWidget(parent), ui(new Ui::MultimediaWidget){
  ui->setupUi(this); //load the designer file

  //Add in the special QMultimediaWidgets
  mediaObj = new QMediaPlayer(this);
    mediaObj->setVolume(100);
    mediaObj->setNotifyInterval(500); //only every 1/2 second update
  videoDisplay = new QVideoWidget(this);
    videoDisplay->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->videoLayout->addWidget(videoDisplay);
    mediaObj->setVideoOutput(videoDisplay);
    videoDisplay->setVisible(false);

  UpdateIcons();
  UpdateText();

  //Connect the special signals/slots for the media object
  connect(mediaObj, SIGNAL(durationChanged(qint64)), this, SLOT(playerDurationChanged(qint64)) );
  connect(mediaObj, SIGNAL(seekableChanged(bool)), ui->playerSlider, SLOT(setEnabled(bool)) );
  connect(mediaObj, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(playerStatusChanged(QMediaPlayer::MediaStatus)) );
  connect(mediaObj, SIGNAL(positionChanged(qint64)), this, SLOT(playerTimeChanged(qint64)) );
  connect(mediaObj, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(playerStateChanged(QMediaPlayer::State)) );
  connect(mediaObj, SIGNAL(videoAvailableChanged(bool)), this, SLOT(playerVideoAvailable(bool)) );
  connect(mediaObj, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(playerError()) );

  //Disable some of the initial states
  ui->tool_player_stop->setEnabled(false); //nothing to stop yet
  ui->tool_player_pause->setVisible(false); //nothing to pause yet
  ui->playerSlider->setEnabled(false); //nothing to seek yet
}

MultimediaWidget::~MultimediaWidget(){

}

// ================
//    PUBLIC SLOTS
// ================
void MultimediaWidget::ClearPlaylist(){
  mediaObj->stop();
  ui->combo_player_list->clear();
}

void MultimediaWidget::LoadMultimedia(QList<LFileInfo> list){
  for(int i=0; i<list.length(); i++){
    if(list[i].isAVFile()){ ui->combo_player_list->addItem(list[i].fileName(), list[i].absoluteFilePath() ); }
  }
}

void MultimediaWidget::Cleanup(){
  mediaObj->stop(); //just make sure the player is stopped
}

//Theme change functions
void MultimediaWidget::UpdateIcons(){
  ui->tool_player_next->setIcon( LXDG::findIcon("media-skip-forward","") );
  ui->tool_player_prev->setIcon( LXDG::findIcon("media-skip-backward","") );
  ui->tool_player_pause->setIcon( LXDG::findIcon("media-playback-pause","") );
  ui->tool_player_play->setIcon( LXDG::findIcon("media-playback-start","") );
  ui->tool_player_stop->setIcon( LXDG::findIcon("media-playback-stop","") );
}

void MultimediaWidget::UpdateText(){
  ui->retranslateUi(this);
}


// =================
//       PRIVATE
// =================
QString MultimediaWidget::msToText(qint64 ms){
  QString disp;
  if(ms>3600000){
    disp.append( QString::number(ms/3600000)+":" );
    ms = ms%3600000;
  }
  if(ms>60000){
    disp.append( QString::number(ms/60000)+":" );
    ms = ms%60000;
  }else{
    disp.append("0:");
  }
  if(ms>1000){
    if(ms>=10000){ disp.append( QString::number(ms/1000) ); }
    else{ disp.append( "0"+QString::number(ms/1000) ); }
  }else{
    disp.append("00");
  }
  return disp;
}

// =================
//    PRIVATE SLOTS
// =================
void MultimediaWidget::playerStatusChanged(QMediaPlayer::MediaStatus stat){
  //Only use this for end-of-file detection - use the state detection otherwise
  if(stat == QMediaPlayer::EndOfMedia){
    if(!mediaObj->isMuted()){ playerFinished(); } //make sure it is not being seeked right now
  }
}

void MultimediaWidget::playerStateChanged(QMediaPlayer::State newstate){
  //This function keeps track of updating the visuals of the player
  bool running = false;
  bool showVideo = false;
  QString msg;
  switch(newstate){
    case QMediaPlayer::PlayingState:
	running=true;
	showVideo = mediaObj->isVideoAvailable();
	msg = "";//mediaObj->metaData(Phonon::TitleMetaData).join(" ");
	if(msg.simplified().isEmpty()){ msg = ui->combo_player_list->currentText(); }
	ui->label_player_novideo->setText(tr("Playing:")+"\n"+msg);
	break;
    case QMediaPlayer::PausedState:
	showVideo=videoDisplay->isVisible(); //don't change the screen
	break;
    case QMediaPlayer::StoppedState:
	ui->label_player_novideo->setText(tr("Stopped"));
        break;
  }
  ui->tool_player_play->setVisible(!running);
  ui->tool_player_pause->setVisible(running);
  ui->tool_player_stop->setEnabled(running);
  ui->label_player_novideo->setVisible(!showVideo);
  videoDisplay->setVisible(showVideo);
}

void MultimediaWidget::playerVideoAvailable(bool showVideo){
  ui->label_player_novideo->setVisible(!showVideo);
  videoDisplay->setVisible(showVideo);
}

void MultimediaWidget::playerDurationChanged(qint64 dur){
    if(dur < 0){ return; } //not ready yet
    ui->playerSlider->setMaximum(mediaObj->duration());
    playerTTime = msToText(dur);
}

void MultimediaWidget::playerTimeChanged(qint64 ctime){
  if(mediaObj->isMuted() || playerTTime.isEmpty() ){ return; } //currently being moved
  ui->playerSlider->setSliderPosition(ctime);
}

void MultimediaWidget::playerError(){
  QString msg = QString(tr("Error Playing File: %1"));
  msg = msg.arg( mediaObj->currentMedia().canonicalUrl().fileName() );
  msg.append("\n"+mediaObj->errorString());
  ui->label_player_novideo->setText(msg);
}

void MultimediaWidget::playerFinished(){
  if(ui->combo_player_list->currentIndex()<(ui->combo_player_list->count()-1) && ui->check_player_gotonext->isChecked()){
    ui->combo_player_list->setCurrentIndex( ui->combo_player_list->currentIndex()+1 );
    QTimer::singleShot(0,this,SLOT(on_tool_player_play_clicked()));
  }else{
    ui->label_player_novideo->setText(tr("Finished"));
  }
}

void MultimediaWidget::on_tool_player_play_clicked(){
  if(mediaObj->state()==QMediaPlayer::PausedState \
      && mediaObj->currentMedia().canonicalUrl().fileName()==ui->combo_player_list->currentText() ){
    mediaObj->play();
  }else{
    mediaObj->stop();
    //Get the selected file path
    QString filePath = ui->combo_player_list->currentData().toString();
      mediaObj->setMedia( QUrl::fromLocalFile(filePath) );
      playerTTime.clear();
      ui->playerSlider->setEnabled(mediaObj->isSeekable());
      mediaObj->play();
  }
}

void MultimediaWidget::on_combo_player_list_currentIndexChanged(int index){
  ui->tool_player_next->setEnabled( ui->combo_player_list->count() > (index+1) );
  ui->tool_player_prev->setEnabled( (index-1) >= 0 );
}

void MultimediaWidget::on_tool_player_next_clicked(){
  ui->combo_player_list->setCurrentIndex( ui->combo_player_list->currentIndex()+1);
  if(mediaObj->state()!=QMediaPlayer::StoppedState){ on_tool_player_play_clicked(); }
}

void MultimediaWidget::on_tool_player_prev_clicked(){
  ui->combo_player_list->setCurrentIndex( ui->combo_player_list->currentIndex()-1);
  if(mediaObj->state()!=QMediaPlayer::StoppedState){ on_tool_player_play_clicked(); }
}

void MultimediaWidget::on_tool_player_pause_clicked(){
  mediaObj->pause();
}

void MultimediaWidget::on_tool_player_stop_clicked(){
  mediaObj->stop();
}

//Slider controls
void MultimediaWidget::on_playerSlider_sliderPressed(){
  mediaObj->setMuted(true);
  mediaObj->pause();
}
void MultimediaWidget::on_playerSlider_sliderReleased(){
  if(mediaObj->state()==QMediaPlayer::PausedState){ mediaObj->play(); }
  mediaObj->setMuted(false);
}
void MultimediaWidget::on_playerSlider_valueChanged(int val){
  ui->label_player_runstats->setText( msToText(val)+"/"+playerTTime );
  if(mediaObj->isMuted()){ mediaObj->setPosition(val); } //currently seeking
}
