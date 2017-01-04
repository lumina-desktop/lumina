//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Henry Hu
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_LDESKTOPBACKGROUND_H_
#define _LUMINA_DESKTOP_LDESKTOPBACKGROUND_H_

#include <QString>
#include <QWidget>
#include <QPixmap>

class LDesktopBackground: public QWidget {
    Q_OBJECT
public:
    LDesktopBackground();
    virtual ~LDesktopBackground();

    virtual void paintEvent(QPaintEvent*);
    static QPixmap setBackground(const QString&, const QString&, QRect geom);

private:
    QPixmap *bgPixmap;
};

#endif // _LUMINA_DESKTOP_LDESKTOPBACKGROUND_H_
