
#include "PhotoView.h"
#include <QDebug>
#include <QWheelEvent>

PhotoView::PhotoView (QWidget *parent) : QGraphicsView (parent)
{
    setCacheMode (CacheBackground);
    setViewportUpdateMode (BoundingRectViewportUpdate);
    setRenderHint (QPainter::HighQualityAntialiasing);
    setTransformationAnchor (AnchorUnderMouse);
}

void PhotoView::zoomIn ()
{
    scale (scaleFactor, scaleFactor);
    isFit = false;
}

void PhotoView::zoomOut ()
{
    scale (invScaleFactor, invScaleFactor);
    isFit = false;
}

void PhotoView::zoomNormal ()
{
    resetMatrix ();
    isFit = false;
}

void PhotoView::zoomFit ()
{
    fitInView (sceneRect (), Qt::KeepAspectRatio);
    isFit = true;
}

bool PhotoView::eventFilter (QObject *, QEvent *event)
{
    if (event->type () == QEvent::Wheel)
    {
        QWheelEvent *wheel_event = static_cast<QWheelEvent *> (event);
        if (wheel_event->delta () > 0)
        {
            if (wheel_event->modifiers () == Qt::ControlModifier)
            {
                emit nextImage ();
            }
            else
            {
                scale (scaleFactor, scaleFactor);
                return true;
            }
        }
        else if (wheel_event->delta () < 0)
        {
            if (wheel_event->modifiers () == Qt::ControlModifier)
            {
                emit prevImage ();
            }
            else
            {
                scale (invScaleFactor, invScaleFactor);
                return true;
            }
        }
    }

    return false;
}

void PhotoView::mouseDoubleClickEvent (QMouseEvent *event)
{
    if (event->button () == Qt::LeftButton)
    {
        if (isFit)
            zoomNormal ();
        else
            zoomFit ();
    }
}
