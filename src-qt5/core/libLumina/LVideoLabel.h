#ifndef LVIDEOLABEL_H 
#define LVIDEOLABEL_H 

#include <QLabel>
#include <QMediaPlayer>
#include "LVideoSurface.h"

//class LVideoSurface;

class LVideoLabel : public QLabel {
  Q_OBJECT
  public:
    LVideoLabel(QString, bool);
    ~LVideoLabel();
  protected:
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
  signals:
    void rollOver();
  public slots:
    void stopVideo(QPixmap);
    void setDuration(QMediaPlayer::MediaStatus);
  private:
    QMediaPlayer *mediaPlayer;
    LVideoSurface *surface;
    QVideoWidget *videoPlayer;
    QPixmap thumbnail;
    bool entered;
};
#endif
