#ifndef VIDNAIL_H
#define VIDNAIL_H

#include <QMediaPlayer>
#include <QtMultimediaWidgets>
#include "videowidgetsurface.h"
#include <QPixmap>
#include <QAbstractVideoSurface>
#include <QImage>
#include <QRect>
#include <QVideoFrame>

public:

  void grabvideothumbnail();

private:

  QAbstractVideoSurface *vsurface;
  QImage::Format imageFormat;
  QPixmap imageCaptured;


#endif // VIDNAIL_H

