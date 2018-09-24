
#include "PhotoView.h"

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
}

void PhotoView::zoomOut ()
{
    scale (invScaleFactor, invScaleFactor);
}

void PhotoView::zoomNormal ()
{
    resetMatrix ();
}

void PhotoView::zoomFit ()
{
    fitInView (sceneRect (), Qt::KeepAspectRatio);
}
