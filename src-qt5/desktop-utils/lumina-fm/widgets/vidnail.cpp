#include "vidnail.h"
vidnail::vidnail(QWidget *parent) : QMainWindow(parent), mplayer(parent, QMediaPlayer::VideoSurface){  //there is no UI, so not sure how to alter the constructor
}

vidnail::~vidnail()
{

vidnail::grabvideothumbnail(){
  vsurface = new QAbstractVideoSurface();
  mplayer.setVideoOutput(vsurface);
  mplayer.setMedia($file);  // video file to get thumbnail of
  imageCaptured = QPixmap();
  mplayer.setPosition(2000); // time in milliseconds
  mplayer.setMuted(true); // just to make sure no sound is emited
  mplayer.play();

  currentFrame = frame;
  const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
  const QSize size = format.frameSize();

  this->imageFormat = imageFormat;
  QAbstractVideoSurface::start(format);
  QImage image( currentFrame.bits(), currentFrame.width(), currentFrame.height(), currentFrame.bytesPerLine(), imageFormat);
  imageCaptured = QPixmap::fromImage(image.copy(image.rect()));

// Now do scaling with regular thumbnail process to make proper size

  mplayer.stop();
  vsurface.stop();
}



