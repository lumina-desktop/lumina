#include "LVideoLabel.h"
#include <LuminaXDG.h>
#include <QCoreApplication>
#include <QTimer>

LVideoLabel::LVideoLabel(QString file, bool icons, QWidget *parent) : QLabel(parent) {
  thumbnail = QPixmap();
  entered = false;
  this->icons = icons;
  filepath = file;
  defaultThumbnail = LXDG::findIcon("media-playback-start", "").pixmap(256,256);
  QTimer::singleShot(qrand()%10, this, SLOT(initializeBackend()) );
}

LVideoLabel::~LVideoLabel() {
  mediaPlayer->deleteLater();
  surface->deleteLater();
}

void LVideoLabel::initializeBackend(){
  mediaPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
  surface = new LVideoSurface(this);
  mediaPlayer->setVideoOutput(surface);
  mediaPlayer->setPlaybackRate(3);
  mediaPlayer->setMuted(true);

  this->setPixmap(defaultThumbnail.scaled(this->size(),Qt::IgnoreAspectRatio));
  mediaPlayer->setMedia(QUrl::fromLocalFile(filepath));

  this->connect(surface, SIGNAL(frameReceived(QPixmap)), this, SLOT(stopVideo(QPixmap)));
  this->connect(mediaPlayer, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChanged(QMediaPlayer::State)));
  this->connect(mediaPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(setDuration(QMediaPlayer::MediaStatus)));
  this->connect(this, SIGNAL(rollOver()), surface, SLOT(switchRollOver()));

  //QTimer::singleShot( qrand()%100,mediaPlayer, SLOT(play()) );
  //mediaPlayer->play();
}

void LVideoLabel::enableIcons() {
  this->setPixmap(thumbnail.scaled(this->size(),Qt::IgnoreAspectRatio));
  icons = true;
}

void LVideoLabel::disableIcons() {
  this->setPixmap(defaultThumbnail.scaled(this->size(),Qt::IgnoreAspectRatio));
  icons = false;
}

void LVideoLabel::stopVideo(QPixmap pix) {
  if(!entered) {
    emit frameReceived(pix);
    if(thumbnail.isNull())
      thumbnail = pix;
    if(icons)
      this->setPixmap(thumbnail.scaled(this->size(),Qt::IgnoreAspectRatio));
    mediaPlayer->pause();
  }else {
    if(icons)
      this->setPixmap(pix.scaled(this->size(),Qt::IgnoreAspectRatio));
  }
}

void LVideoLabel::stateChanged(QMediaPlayer::State state) {
  //qDebug() << state;
}

void LVideoLabel::setDuration(QMediaPlayer::MediaStatus status) {
  //qDebug() << status;
  if(status == QMediaPlayer::BufferedMedia && !entered) { //Set duration in the middle to capture the thumbnail
    mediaPlayer->setPosition(mediaPlayer->duration() / 2);
    mediaPlayer->play();
  }else if(status == QMediaPlayer::EndOfMedia && entered) { //Loop back to the beginning if playback started and at the end of the video
    mediaPlayer->setPosition(0);
    mediaPlayer->play();
  }else if(status == QMediaPlayer::InvalidMedia){
    mediaPlayer->stop();
    QTimer::singleShot(qrand()%100, mediaPlayer, SLOT(play())); //mediaPlayer->play();
  }/*else if(status == QMediaPlayer::LoadingMedia) {
    mediaPlayer->pause();
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(300);
    timer.start();
    qDebug() << "Timer Started" << timer.remainingTime();
    while(timer.isActive()) QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
    qDebug() << "Timer Finished" << timer.remainingTime();
    mediaPlayer->setPosition(0);
    mediaPlayer->play();
  }*/
}

void LVideoLabel::resizeEvent(QResizeEvent *event) {
  //Resize the current pixmap to match the new size
  if(!thumbnail.isNull()){
    if(icons)
      this->setPixmap(thumbnail.scaled(this->size(),Qt::IgnoreAspectRatio));
    else
      this->setPixmap(defaultThumbnail.scaled(this->size(),Qt::IgnoreAspectRatio));
  }
  QLabel::resizeEvent(event);
}

//Start playing the video from the beginning when the mouse enters the label
void LVideoLabel::enterEvent(QEvent *event) {
  if(icons) {
    entered=true;
    emit rollOver();
    mediaPlayer->setPosition(0);
    mediaPlayer->play();
  }
  QWidget::enterEvent(event);
}

//Stop the video and set the thumbnail back to the middle of the video when the mouse leaves the label
void LVideoLabel::leaveEvent(QEvent *event) {
  if(icons) {
    entered=false;
    mediaPlayer->setPosition(mediaPlayer->duration() / 2);
    emit rollOver();
  }
  QWidget::leaveEvent(event);
}
