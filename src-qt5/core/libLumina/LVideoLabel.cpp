#include "LVideoLabel.h"
#include <QTimer>
LVideoLabel::LVideoLabel(QString file, QWidget *parent) : QLabel(parent) {
  this->setScaledContents(true);
  this->setPixmap(thumbnail); //blank pixmap by default
  entered = false;
  filepath = file;
  QTimer::singleShot(0, this, SLOT(initializeBackend()) );
}

LVideoLabel::~LVideoLabel() {

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
    this->setPixmap((shrink) ? thumbnail.scaledToHeight(64) : thumbnail);
    mediaPlayer->pause();
  }else {
    this->setPixmap((shrink) ? pix.scaledToHeight(64) : pix);
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
