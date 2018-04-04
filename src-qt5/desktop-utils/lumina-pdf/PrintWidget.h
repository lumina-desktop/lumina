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
#include <QTextDocument>
#include "Renderer.h"
#include "TextData.h"

class AnnotItem: public QGraphicsItem {
public:
  AnnotItem(QGraphicsItem *parent, QList<QString> textData, QRectF loc) : QGraphicsItem(parent), author(textData[0]), text(textData[1]) {
    setCacheMode(DeviceCoordinateCache);
    QString allText = "Author: " + author + "\n\n" + text;
    QTextDocument document;
    document.setDefaultFont(QFont("Helvitica", 10, QFont::Normal));
    document.setPageSize(QSize(120, 120));
    document.setHtml(allText);
    loc.moveTopLeft(QPointF(loc.center().x(), loc.center().y()+loc.height()/2));
    loc.setSize(document.size()+QSize(10, 10));
    bbox = loc;
  }

  QRectF boundingRect() const Q_DECL_OVERRIDE { return bbox; }

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE
  {
    Q_UNUSED(widget);
    QString allText = "Author: " + author + "\n\n" + text;
    painter->setClipRect(option->exposedRect);
    painter->setFont(QFont("Helvitica", 10, QFont::Normal));
    painter->setBrush(QBrush(QColor(255, 255, 177, 255)));
    painter->drawRect(bbox);
    painter->setPen(QPen(QColor("Black")));
    painter->drawText(bbox, Qt::AlignLeft | Qt::TextWordWrap, allText);
  }

private:
  QRectF bbox;
  QString author;
  QString text;
};

class AnnotZone: public QGraphicsItem {
public:
  AnnotZone(QGraphicsItem *parent, QRectF _bbox, AnnotItem *_annot) : QGraphicsItem(parent), bbox(_bbox), annot(_annot) { }
  QRectF boundingRect() const Q_DECL_OVERRIDE { return bbox; }
  AnnotItem* annotation() const { return annot; }
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE { Q_UNUSED(widget); Q_UNUSED(painter); Q_UNUSED(option); } 

private:
  QRectF bbox;
  AnnotItem *annot;
};

class LinkItem: public QGraphicsItem {
public:
  LinkItem(QGraphicsItem *parent, TextData *_data) : QGraphicsItem(parent), bbox(_data->loc()), data(_data) {
    setCacheMode(DeviceCoordinateCache);
    setAcceptHoverEvents(true);
  }

QRectF boundingRect() const Q_DECL_OVERRIDE
  { return bbox; }

inline TextData* getData() const
  { return data; }

void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE
{
  Q_UNUSED(widget);
  painter->setClipRect(option->exposedRect);
  painter->setBrush(QBrush(QColor(255, 255, 177, 100)));
  painter->setPen(QPen(QColor(255, 255, 100, 125)));
  painter->drawRect(bbox);
}

private:
  QRectF bbox;
  TextData *data;
};

class PageItem : public QGraphicsItem {
public:
  PageItem(int _pageNum, QImage _pagePicture, QSize _paperSize)
        : pageNum(_pageNum), pagePicture(_pagePicture), paperSize(_paperSize)
  {
    brect = QRectF(QPointF(-25, -25),
              QSizeF(paperSize)+QSizeF(50, 50));
    setCacheMode(DeviceCoordinateCache);
    setAcceptHoverEvents(true);
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
  int curPage, publicPageNum;
  ViewMode viewMode;
  ZoomMode zoomMode;
  QPageLayout::Orientation orientation;
  double zoomFactor;
  bool initialized, fitting;
  QList<QGraphicsItem*> pages;
  QHash<int, QList<QGraphicsItem*>> links;
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
  void goToPosition(int, float, float);

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

  void clearItems(QList<QGraphicsItem*> itemList, QGraphicsItem *item) {
    foreach(QGraphicsItem *graphicsItem, itemList) {
      if(item == graphicsItem)
        continue;
      if(graphicsItem == dynamic_cast<LinkItem*>(graphicsItem))
        graphicsItem->setOpacity(0.1);
      if(graphicsItem == dynamic_cast<AnnotItem*>(graphicsItem))
        graphicsItem->setVisible(false);
    }
  }

  void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE {
    QGraphicsView::mouseMoveEvent(e);

    if(QGraphicsItem *item = scene->itemAt(mapToScene(e->pos()), transform())) {
      QList<QGraphicsItem*> linkList;
      if(item == dynamic_cast<AnnotItem*>(item))
        item = item->parentItem();

      if(PageItem *page = dynamic_cast<PageItem*>(item))
        linkList = page->childItems();
      else
        linkList = item->parentItem()->childItems();

      if(LinkItem *link = dynamic_cast<LinkItem*>(item)){
        item->setOpacity(1);
      }else if(AnnotZone *annotZone = dynamic_cast<AnnotZone*>(item)){
        annotZone->annotation()->setVisible(true);
        item = annotZone->annotation();
      }

      clearItems(linkList, item);
    }
  }

  void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE {
    QGraphicsView::mouseReleaseEvent(e);
    QPointF scenePoint = mapToScene(e->pos());
    QGraphicsItem *item = scene->itemAt(scenePoint, transform());
    if(LinkItem *link = dynamic_cast<LinkItem*>(item)) {
      PageItem *page = dynamic_cast<PageItem*>(link->parentItem());
      if(!BACKEND->isExternalLink(page->pageNumber()-1, link->getData()->text())) {
        BACKEND->handleLink(link->getData()->text());
      }else{
        ExternalLinkDialog
      } 
      link->setOpacity(0.1);
    }
  }

  void showEvent(QShowEvent* e) Q_DECL_OVERRIDE {
    QGraphicsView::showEvent(e);
    emit resized();
  }
};
#endif
