#include "LVideoSurface.h"
#include <QDebug>

LVideoSurface::LVideoSurface() : QAbstractVideoSurface() {
  recording = 0;
  frameImage = QImage();
}

QImage LVideoSurface::currentFrame() {
  return frameImage;
}

bool LVideoSurface::present(const QVideoFrame &frame) {
  if(frame.isValid()) {
    qDebug() << "Recording Frame" << frame.pixelFormat();
    QVideoFrame icon(frame);
    icon.map(QAbstractVideoBuffer::ReadOnly);
    frameImage = QImage(icon.bits(), icon.width(), icon.height(), icon.bytesPerLine(), QImage::Format_ARGB32_Premultiplied);
    icon.unmap();
    emit frameReceived(frameImage);
    if(recording++ == 2) ready = true;
    return true;
  }
  ready = false;
  return false;
}

QList<QVideoFrame::PixelFormat> LVideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const {
  Q_UNUSED(type);
  return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_ARGB32 << QVideoFrame::Format_RGB32 << QVideoFrame::Format_RGB24
         << QVideoFrame::Format_RGB565 << QVideoFrame::Format_RGB555 << QVideoFrame::Format_BGRA32 << QVideoFrame::Format_BGR32;
}
