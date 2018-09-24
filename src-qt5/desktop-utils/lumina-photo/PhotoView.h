#pragma once

#include <QGraphicsView>

class PhotoView : public QGraphicsView
{
    Q_OBJECT
    public:
        PhotoView (QWidget *parent = nullptr);
        void zoomNormal ();
        void zoomFit ();
        void zoomIn ();
        void zoomOut ();

    private:
        const double scaleFactor = 1.25;
        const double invScaleFactor = 1 / 1.25;
};
