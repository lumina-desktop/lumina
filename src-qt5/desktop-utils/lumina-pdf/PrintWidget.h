//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// Simple subclass of QPrintPreviewWidget to provide
// notification when a context menu is requested
//===========================================
#ifndef _PRINT_GRAPHICS_H
#define _PRINT_GRAPHICS_H

#include <QMouseEvent>
#include <QDebug>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QBoxLayout>
#include <QScrollBar>
#include <QStyleOptionGraphicsItem>
#include <QtMath>
#include <QPageLayout>
#include "Renderer.h"

#include "textData.h"

class PageItem : public QGraphicsItem {
public:
  PageItem(int _pageNum, QImage _pagePicture, QSize _paperSize)
        : pageNum(_pageNum), pagePicture(_pagePicture), paperSize(_paperSize)
  {
    brect = QRectF(QPointF(-25, -25),
              QSizeF(paperSize)+QSizeF(50, 50));
    setCacheMode(DeviceCoordinateCache);
  }

  QRectF boundingRect() const Q_DECL_OVERRIDE
    { return brect; }

  inline int pageNumber() const
    { return pageNum; }

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE
  {
    Q_UNUSED(widget);
    //Ensure all the antialiasing/smoothing options are turned on
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    QRectF paperRect(0,0, paperSize.width(), paperSize.height());
    // Draw shadow
    painter->setClipRect(option->exposedRect);
    qreal shWidth = paperRect.width()/100;
    QRectF rshadow(paperRect.topRight() + QPointF(0, shWidth),
           paperRect.bottomRight() + QPointF(shWidth, 0));
    QLinearGradient rgrad(rshadow.topLeft(), rshadow.topRight());
    rgrad.setColorAt(0.0, QColor(0,0,0,255));
    rgrad.setColorAt(1.0, QColor(0,0,0,0));
    painter->fillRect(rshadow, QBrush(rgrad));
    QRectF bshadow(paperRect.bottomLeft() + QPointF(shWidth, 0),
           paperRect.bottomRight() + QPointF(0, shWidth));
    QLinearGradient bgrad(bshadow.topLeft(), bshadow.bottomLeft());
    bgrad.setColorAt(0.0, QColor(0,0,0,255));
    bgrad.setColorAt(1.0, QColor(0,0,0,0));
    painter->fillRect(bshadow, QBrush(bgrad));
    QRectF cshadow(paperRect.bottomRight(),
           paperRect.bottomRight() + QPointF(shWidth, shWidth));
    QRadialGradient cgrad(cshadow.topLeft(), shWidth, cshadow.topLeft());
    cgrad.setColorAt(0.0, QColor(0,0,0,255));
    cgrad.setColorAt(1.0, QColor(0,0,0,0));
    painter->fillRect(cshadow, QBrush(cgrad));
    painter->setClipRect(paperRect & option->exposedRect);
    painter->fillRect(paperRect, Qt::white);
    painter->drawImage(QPoint(0,0), pagePicture);
  }

private:
  int pageNum;
  QImage pagePicture;
  QSize paperSize;
  QRectF brect;
};


class PrintWidget : public QGraphicsView
{
  Q_OBJECT
public:
  enum ViewMode {
    SinglePageView,
    FacingPagesView,
    AllPagesView
  };

  enum ZoomMode {
    CustomZoom,
    FitToWidth,
    FitInView
  };

private:
  void generatePreview();
  void layoutPages();
  void populateScene();
  void setViewMode(ViewMode);
  void setZoomMode(ZoomMode);

  QGraphicsScene *scene;
  QMatrix rotMatrix;
  int curPage, publicPageNum;
  ViewMode viewMode;
  ZoomMode zoomMode;
  QPageLayout::Orientation orientation;
  double zoomFactor;
  bool initialized, fitting;
  QList<QGraphicsItem*> pages;
  int degrees;
  Renderer *BACKEND;

public:
  PrintWidget(Renderer *backend, QWidget *parent = 0);
  ~PrintWidget();

  double getZoomFactor() const { return this->zoomFactor; };
  ZoomMode getZoomMode() const { return this->zoomMode; };
  int currentPage() const { return publicPageNum; };

signals:
  void resized();
  void customContextMenuRequested(const QPoint&);
  void currentPageChanged();

public slots:
  void zoomIn(double factor=1.2);
  void zoomOut(double factor=1.2);
  void setCurrentPage(int);
  void setVisible(bool) Q_DECL_OVERRIDE;
  void highlightText(TextData*);
  void setDegrees(int);

  void updatePreview();
  void fitView();
  void fitToWidth();
  void setAllPagesViewMode();
  void setSinglePageViewMode();
  void setFacingPagesViewMode();

private slots:
  void updateCurrentPage();
  int calcCurrentPage();
  void fit(bool doFitting=false);

protected:
  void resizeEvent(QResizeEvent* e) Q_DECL_OVERRIDE {
    /*{
      const QSignalBlocker blocker(verticalScrollBar()); // Don't change page, QTBUG-14517
      QGraphicsView::resizeEvent(e);
    }*/
    QGraphicsView::resizeEvent(e);
    emit resized();
   }

  void showEvent(QShowEvent* e) Q_DECL_OVERRIDE {
    QGraphicsView::showEvent(e);
    emit resized();
  }
};
#endif
