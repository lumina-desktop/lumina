#include "LVideoLabel.h"

LVideoLabel::LVideoLabel(QString file, bool video) : QLabel(){ 
    this->setScaledContents(true);
    if(video) {
      mediaPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
      thumbnail = QPixmap();
      entered = false;
      surface = new LVideoSurface(this);
      mediaPlayer->setVideoOutput(surface);
      mediaPlayer->setMedia(QUrl("file://" + file));
      mediaPlayer->setPlaybackRate(3);
      mediaPlayer->setMuted(true);
      mediaPlayer->play();
      mediaPlayer->pause();
      this->connect(surface, SIGNAL(frameReceived(QPixmap)), this, SLOT(stopVideo(QPixmap)));
      this->connect(mediaPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(setDuration(QMediaPlayer::MediaStatus)));
      this->connect(this, SIGNAL(rollOver()), surface, SLOT(switchRollOver()));
    }else{
      thumbnail = QPixmap(file);
      this->setPixmap(thumbnail);
    }
}

LVideoLabel::~LVideoLabel() {
  mediaPlayer->deleteLater();
  surface->deleteLater();
}

void LVideoLabel::stopVideo(QPixmap pix) {
  if(!entered) {
    if(thumbnail.isNull())
      thumbnail = QPixmap(pix.scaledToHeight(64));
    this->setPixmap(thumbnail);
    mediaPlayer->pause();
  }else {
    this->setPixmap(QPixmap(pix.scaledToHeight(64)));
  }
}

void LVideoLabel::setDuration(QMediaPlayer::MediaStatus status) {
  if(status == QMediaPlayer::BufferedMedia) {
    mediaPlayer->setPosition(mediaPlayer->duration() / 2);
    mediaPlayer->play();
  }
}

void LVideoLabel::enterEvent(QEvent *event) {
  entered=true;
  emit rollOver();
  mediaPlayer->setPosition(0);
  mediaPlayer->play();
  QLabel::enterEvent(event);
}

void LVideoLabel::leaveEvent(QEvent *event) {
  entered=false;
  mediaPlayer->setPosition(mediaPlayer->duration() / 2);
  emit rollOver();
  QLabel::leaveEvent(event);
}
