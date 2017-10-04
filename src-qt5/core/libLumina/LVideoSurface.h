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
    /*virtual QList<QVidebool isFormatSupported(const QVideoSurfaceFormat &format) const;
    bool start(const QVideoSurfaceFormat &format);
    void stop();*/
  signals:
    void frameReceived(QPixmap);
  private:
    QPixmap frameImage;
    //QImage::Format imageFormat;
};
