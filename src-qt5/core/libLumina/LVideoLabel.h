#ifndef LVIDEOLABEL_H
#define LVIDEOLABEL_H

#include <QLabel>
#include <QMediaPlayer>
#include <QTimer>
#include <QResizeEvent>
#include "LVideoSurface.h"

class LVideoLabel : public QLabel{
  Q_OBJECT
  public:
    LVideoLabel(QString, bool, QWidget* parent=NULL);
    ~LVideoLabel();
    void enableIcons();
    void disableIcons();

  protected:
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    void resizeEvent(QResizeEvent*);

  signals:
    void rollOver();
    void frameReceived(QPixmap);

  private slots:
    void initializeBackend();
    void stopVideo(QPixmap);
    void setDuration(QMediaPlayer::MediaStatus);
    void stateChanged(QMediaPlayer::State);

  private:
    QMediaPlayer *mediaPlayer;
    LVideoSurface *surface;
    QPixmap thumbnail;
    QPixmap defaultThumbnail;
    bool entered;
    bool icons;
    QString filepath;
};
#endif
