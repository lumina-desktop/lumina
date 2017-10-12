#ifndef LVIDEOSURFACE_H
#define LVIDEOSURFACE_H

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
#endif
