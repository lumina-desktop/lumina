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

#include "LSession.h"

void LDesktopBackground::paintEvent(QPaintEvent *ev) {
  //return; //do nothing - always invisible
    if (bgPixmap != NULL) {
        //qDebug() << "Wallpaper paint Event:" << ev->rect();
        //QPainter painter(this);
        //painter.setBrush(*bgPixmap);
        //painter.drawRect(ev->rect().adjusted(-1,-1,2,2));
    }else{
      QWidget::paintEvent(ev);
   }
}

QPixmap LDesktopBackground::setBackground(const QString& bgFile, const QString& format, QRect geom) {
    //if (bgPixmap != NULL) delete bgPixmap;
    QPixmap bgPixmap(geom.size());// = new QPixmap(size());

    if (bgFile.startsWith("rgb(")) {
        QStringList colors = bgFile.section(")",0,0).section("(",1,1).split(",");
        QColor color = QColor(colors[0].toInt(), colors[1].toInt(), colors[2].toInt());
        bgPixmap.fill(color);
    } else {
        bgPixmap.fill(Qt::black);

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
            if(bgImage.height() != geom.height() && bgImage.width() != geom.width() ){ bgImage = bgImage.scaled(geom.size(), mode, Qt::SmoothTransformation);  }
            //bgImage = bgImage.scaled(size(), mode);
        }

        // Calculate the offset
        int dx = 0, dy = 0;
        int drawWidth = bgImage.width(), drawHeight = bgImage.height();
        if (format == "fit" || format == "center" || format == "full") {
            dx = (geom.width() - bgImage.width()) / 2;
            dy = (geom.height() - bgImage.height()) / 2;
        } else if (format == "tile") {
            drawWidth = geom.width();
            drawHeight = geom.height();
        } else {
            if (format.endsWith("right")) {
                dx = geom.width() - bgImage.width();
            }
            if (format.startsWith("bottom")) {
                dy = geom.height() - bgImage.height();
            }
        }

        // Draw the background image
        QPainter painter(&bgPixmap);
        painter.setBrush(bgImage);
        painter.setBrushOrigin(dx, dy);
        painter.drawRect(dx, dy, drawWidth, drawHeight);
    }
    //this->repaint(); //make sure the entire thing gets repainted right away
   //LSession::handle()->XCB->paintRoot(geom, &bgPixmap);
    return bgPixmap;
    //show();
}

LDesktopBackground::LDesktopBackground() : QWidget() {
    bgPixmap = NULL;
    this->setWindowOpacity(0);
}

LDesktopBackground::~LDesktopBackground() {
    if (bgPixmap != NULL) delete bgPixmap;
}
