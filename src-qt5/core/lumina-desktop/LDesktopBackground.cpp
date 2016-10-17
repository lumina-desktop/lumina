//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Henry Hu
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LDesktopBackground.h"

#include <QPainter>
#include <QPaintEvent>
#include <QDebug>

void LDesktopBackground::paintEvent(QPaintEvent *ev) {
    if (bgPixmap != NULL) {
        //qDebug() << "Wallpaper paint Event:" << ev->rect();
        QPainter painter(this);
        painter.setBrush(*bgPixmap);
        painter.drawRect(ev->rect().adjusted(-1,-1,2,2));
    }else{
      QWidget::paintEvent(ev);
   }
}

void LDesktopBackground::setBackground(const QString& bgFile, const QString& format) {
    if (bgPixmap != NULL) delete bgPixmap;
    bgPixmap = new QPixmap(size());

    if (bgFile.startsWith("rgb(")) {
        QStringList colors = bgFile.section(")",0,0).section("(",1,1).split(",");
        QColor color = QColor(colors[0].toInt(), colors[1].toInt(), colors[2].toInt());
        bgPixmap->fill(color);
    } else {
        bgPixmap->fill(Qt::black);

        // Load the background file and scale
        QPixmap bgImage(bgFile);
        if (format == "stretch" || format == "full" || format == "fit") {
            Qt::AspectRatioMode mode;
            if (format == "stretch") {
                mode = Qt::IgnoreAspectRatio;
            } else if (format == "full") {
                mode = Qt::KeepAspectRatioByExpanding;
            } else {
                mode = Qt::KeepAspectRatio;
            }
            if(bgImage.height() != this->height() && bgImage.width() != this->width() ){ bgImage = bgImage.scaled(size(), mode);  }
            //bgImage = bgImage.scaled(size(), mode);
        }

        // Calculate the offset
        int dx = 0, dy = 0;
        int drawWidth = bgImage.width(), drawHeight = bgImage.height();
        if (format == "fit" || format == "center" || format == "full") {
            dx = (width() - bgImage.width()) / 2;
            dy = (height() - bgImage.height()) / 2;
        } else if (format == "tile") {
            drawWidth = width();
            drawHeight = height();
        } else {
            if (format.endsWith("right")) {
                dx = width() - bgImage.width();
            }
            if (format.startsWith("bottom")) {
                dy = height() - bgImage.height();
            }
        }

        // Draw the background image
        QPainter painter(bgPixmap);
        painter.setBrush(bgImage);
        painter.setBrushOrigin(dx, dy);
        painter.drawRect(dx, dy, drawWidth, drawHeight);
    }
    this->repaint(); //make sure the entire thing gets repainted right away
    show();
}

LDesktopBackground::LDesktopBackground() : QWidget() {
    bgPixmap = NULL;
}

LDesktopBackground::~LDesktopBackground() {
    if (bgPixmap != NULL) delete bgPixmap;
}
