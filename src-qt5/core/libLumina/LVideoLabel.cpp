#include "LVideoLabel.h"

LVideoLabel::LVideoLabel(QString file, QWidget *parent) : QLabel(parent) {
  thumbnail = QPixmap();
  entered = false;
  shrink = true;
  filepath = file;

  QTimer::singleShot(0, this, SLOT(initializeBackend()) );
}

LVideoLabel::~LVideoLabel() {
  mediaPlayer->deleteLater();
  surface->deleteLater();
}

void LVideoLabel::setShrinkPixmap(bool shrink) {
  this->shrink = shrink;
}

void LVideoLabel::initializeBackend(){
  mediaPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
  surface = new LVideoSurface(this);
  mediaPlayer->setVideoOutput(surface);
  mediaPlayer->setPlaybackRate(3);
  mediaPlayer->setMuted(true);
  
  mediaPlayer->setMedia(QUrl::fromLocalFile(filepath));
  mediaPlayer->play();
  mediaPlayer->pause();

  this->connect(surface, SIGNAL(frameReceived(QPixmap)), this, SLOT(stopVideo(QPixmap)));
  this->connect(mediaPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(setDuration(QMediaPlayer::MediaStatus)));
  this->connect(this, SIGNAL(rollOver()), surface, SLOT(switchRollOver()));
}

void LVideoLabel::stopVideo(QPixmap pix) {
  if(!entered) {
    emit frameReceived(pix);
    if(thumbnail.isNull())
      thumbnail = pix;
    this->setPixmap(thumbnail.scaled(this->size(),Qt::IgnoreAspectRatio));
    mediaPlayer->pause();
  }else {
    this->setPixmap(pix.scaled(this->size(),Qt::IgnoreAspectRatio));
  }
}

void LVideoLabel::setDuration(QMediaPlayer::MediaStatus status) {
  if(status == QMediaPlayer::BufferedMedia) {
    mediaPlayer->setPosition(mediaPlayer->duration() / 2);
    mediaPlayer->play();
  }
}

void LVideoLabel::resizeEvent(QResizeEvent *event) {
  if(!thumbnail.isNull())
    this->setPixmap(thumbnail.scaled(this->size(),Qt::IgnoreAspectRatio));
  QLabel::resizeEvent(event);
}

void LVideoLabel::enterEvent(QEvent *event) {
  entered=true;
  emit rollOver();
  mediaPlayer->setPosition(0);
  mediaPlayer->play();
  QWidget::enterEvent(event);
}

void LVideoLabel::leaveEvent(QEvent *event) {
  entered=false;
  mediaPlayer->setPosition(mediaPlayer->duration() / 2);
  emit rollOver();
  QWidget::leaveEvent(event);
}
