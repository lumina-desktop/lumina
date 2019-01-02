//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// Simple subclass of QLabel to provide
// some overlay information as a presentation window
//===========================================
#ifndef _PRESENTATION_LABEL_WIDGET_H
#define _PRESENTATION_LABEL_WIDGET_H

#include <QDebug>
#include <QLabel>
#include <QMouseEvent>

class PresentationLabel : public QLabel {
  Q_OBJECT

signals:
  void nextSlide();

public:
  PresentationLabel()
      : QLabel(0, Qt::Window | Qt::FramelessWindowHint |
                      Qt::WindowStaysOnTopHint) {
    this->setContextMenuPolicy(Qt::CustomContextMenu);
  }

protected:
  void mousePressEvent(QMouseEvent *ev) {
    QLabel::mousePressEvent(ev);
    if (ev->button() == Qt::LeftButton) {
      emit nextSlide();
    }
  }
};

#endif
