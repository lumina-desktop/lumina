#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QDebug>

class LVideoSurface : public QAbstractVideoSurface {
  Q_OBJECT

  public:
    LVideoSurface();
    virtual bool present(const QVideoFrame&);
    virtual QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType) const;
    QImage currentFrame();
    bool frameReady();
  signals:
    void frameReceived(QImage);
  private:
    int recording;
    QImage frameImage;
    bool ready;
};
