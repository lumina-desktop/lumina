#ifndef LVIDEOLABEL_H 
#define LVIDEOLABEL_H 

#include <QLabel>
#include <QMediaPlayer>
#include "LVideoSurface.h"

//class LVideoSurface;

class LVideoLabel : public QLabel {
  Q_OBJECT
  public:
    LVideoLabel(QString, QWidget* parent=NULL);
    ~LVideoLabel();
    void setShrinkPixmap(bool);
    
  protected:
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
  signals:
    void rollOver();
    void frameReceived(QPixmap);
  private slots:
    void stopVideo(QPixmap);
    void setDuration(QMediaPlayer::MediaStatus);
  private:
    QMediaPlayer *mediaPlayer;
    LVideoSurface *surface;
    QPixmap thumbnail;
    bool entered;
    bool shrink;
};
#endif
