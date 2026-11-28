#ifndef LVIDEOSURFACE_H
#define LVIDEOSURFACE_H

#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
// QAbstractVideoSurface was removed in Qt6, replaced by QVideoSink.
// This class is only available in Qt5 builds.

#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QPixmap>
#include <QDebug>

class LVideoSurface : public QAbstractVideoSurface {
  Q_OBJECT

  public:
    LVideoSurface(QObject *parent=0);
    virtual bool present(const QVideoFrame&);
    virtual QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType) const;
    bool start(const QVideoSurfaceFormat &format);
    void stop();
  signals:
    void frameReceived(QPixmap);
  public slots:
    void switchRollOver();
  private:
    QPixmap frameImage;
    bool entered;
};

#endif // QT_VERSION < 6.0.0
#endif // LVIDEOSURFACE_H
