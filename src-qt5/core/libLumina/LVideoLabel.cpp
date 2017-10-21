#include "LVideoLabel.h"
#include <QCoreApplication>

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

  this->connect(surface, SIGNAL(frameReceived(QPixmap)), this, SLOT(stopVideo(QPixmap)));
  this->connect(mediaPlayer, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChanged(QMediaPlayer::State)));
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
    mediaPlayer->play();
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
  if(!thumbnail.isNull()) //Resize the current pixmap to match the new size
    this->setPixmap(thumbnail.scaled(this->size(),Qt::IgnoreAspectRatio));
  QLabel::resizeEvent(event);
}

//Start playing the video from the beginning when the mouse enters the label
void LVideoLabel::enterEvent(QEvent *event) {
  entered=true;
  emit rollOver();
  mediaPlayer->setPosition(0);
  mediaPlayer->play();
  QWidget::enterEvent(event);
}

//Stop the video and set the thumbnail back to the middle of the video when the mouse leaves the label
void LVideoLabel::leaveEvent(QEvent *event) {
  entered=false;
  mediaPlayer->setPosition(mediaPlayer->duration() / 2);
  emit rollOver();
  QWidget::leaveEvent(event);
}
