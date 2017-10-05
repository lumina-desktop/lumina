#include "LVideoSurface.h"
#include <QDebug>

LVideoSurface::LVideoSurface(QObject *parent) : QAbstractVideoSurface(parent) {
  frameImage = QPixmap();
}

bool LVideoSurface::present(const QVideoFrame &frame) {
  if(!frameImage.isNull()) {
    emit frameReceived(frameImage);
    return true;
  }

  if(frame.isValid()) {
    //qDebug() << "Recording Frame" << frame.pixelFormat();
    QVideoFrame icon(frame);
    icon.map(QAbstractVideoBuffer::ReadOnly);
    QImage img(icon.bits(), icon.width(), icon.height(), icon.bytesPerLine(), QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat()));
    
    if(frameImage.isNull())
      frameImage = QPixmap::fromImage(img.copy(img.rect()));

    icon.unmap();
    emit frameReceived(frameImage);
    return true;
  }
  return false;
}

QList<QVideoFrame::PixelFormat> LVideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const {
  Q_UNUSED(type);
  return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_ARGB32 << QVideoFrame::Format_RGB32 << QVideoFrame::Format_RGB24
         << QVideoFrame::Format_RGB565 << QVideoFrame::Format_RGB555 << QVideoFrame::Format_BGRA32 << QVideoFrame::Format_BGR32;
}

/*bool VideoSurface::isFormatSupported(const QVideoSurfaceFormat &format) const {
  const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
  const QSize size = format.frameSize();

  return imageFormat != QImage::Format_Invalid && !size.isEmpty() && format.handleType() == QAbstractVideoBuffer::NoHandle;
}

void VideoSurface::stop() {
  QAbstractVideoSurface::stop();
}

bool VideoSurface::start(const QVideoSurfaceFormat &format) {
  const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
  const QSize size = format.frameSize();

  if (imageFormat != QImage::Format_Invalid && !size.isEmpty()) {
      this->imageFormat = imageFormat;
      QAbstractVideoSurface::start(format);
      return true;
  } else {
      return false;
  }
}*/
