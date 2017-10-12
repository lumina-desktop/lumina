#include "LVideoSurface.h"
#include <QDebug>

LVideoSurface::LVideoSurface(QObject *parent) : QAbstractVideoSurface(parent) {
  frameImage = QPixmap();
  entered = false;
}

bool LVideoSurface::present(const QVideoFrame &frame) {
  //qDebug() << surfaceFormat().pixelFormat() << frame.pixelFormat() << surfaceFormat().frameSize() << frame.size();
  if(!frameImage.isNull() && !entered) {
    emit frameReceived(frameImage);
    return true;
  }

  if(frame.isValid()) {
    //qDebug() << "Recording Frame" << frame.pixelFormat();
    QVideoFrame icon(frame);
    icon.map(QAbstractVideoBuffer::ReadOnly);
    //qDebug() << icon.width() << icon.height();
    QImage img(icon.bits(), icon.width(), icon.height(), icon.bytesPerLine(), QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat()));
    
    if((frameImage.isNull() && !entered) or entered)
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

void LVideoSurface::stop() {
  QAbstractVideoSurface::stop();
}

void LVideoSurface::switchRollOver() {
  entered = !entered;
}

bool LVideoSurface::start(const QVideoSurfaceFormat &format) {
  const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
  const QSize size = format.frameSize();

  if (imageFormat != QImage::Format_Invalid && !size.isEmpty()) {
      QAbstractVideoSurface::start(format);
      return true;
  } else {
      return false;
  }
}
