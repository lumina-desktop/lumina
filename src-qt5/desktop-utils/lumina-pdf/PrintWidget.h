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

#include "Renderer.h"
#include "TextData.h"
#include "graphicsitems.h"
#include <QApplication>
#include <QBoxLayout>
#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QPageLayout>
#include <QScrollBar>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>
#include <QtMath>

class PrintWidget : public QGraphicsView {
  Q_OBJECT
public:
  enum ViewMode { SinglePageView, FacingPagesView, AllPagesView };

  enum ZoomMode { CustomZoom, FitToWidth, FitInView };

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
  QList<QGraphicsItem *> pages;
  QHash<int, QList<QGraphicsItem *>> links;
  QHash<int, QList<QGraphicsItem *>> annots;
  int degrees;
  Renderer *BACKEND;

public:
  PrintWidget(Renderer *backend, QWidget *parent = 0);
  ~PrintWidget();

  double getZoomFactor() const { return this->zoomFactor; }
  ZoomMode getZoomMode() const { return this->zoomMode; }
  int currentPage() const {
    return curPage;
    //return publicPageNum;
  }

signals:
  void resized();
  void customContextMenuRequested(const QPoint &);
  void currentPageChanged();

public slots:
  void zoomIn(double factor = 1.2);
  void zoomOut(double factor = 1.2);
  void setCurrentPage(int);
  void setVisible(bool) Q_DECL_OVERRIDE;
  void highlightText(TextData *);
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
  void fit(bool doFitting = false);

protected:
  void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE {
    /*{
      const QSignalBlocker blocker(verticalScrollBar()); // Don't change page,
    QTBUG-14517 QGraphicsView::resizeEvent(e);
    }*/
    QGraphicsView::resizeEvent(e);
    emit resized();
  }

  void clearItems(QList<QGraphicsItem *> itemList, QGraphicsItem *item) {
    foreach (QGraphicsItem *graphicsItem, itemList) {
      if (item == graphicsItem)
        continue;
      if (graphicsItem == dynamic_cast<LinkItem *>(graphicsItem))
        graphicsItem->setOpacity(0.1);
      if (graphicsItem == dynamic_cast<PopupItem *>(graphicsItem))
        graphicsItem->setVisible(false);
    }
  }

  void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE {
    QGraphicsView::mouseMoveEvent(e);
    static bool cursorSet = false;

    if (QGraphicsItem *item =
            scene->itemAt(mapToScene(e->pos()), transform())) {
      QList<QGraphicsItem *> linkList;
      if (item == dynamic_cast<PopupItem *>(item))
        item = item->parentItem();

      if (PageItem *page = dynamic_cast<PageItem *>(item))
        linkList = page->childItems();
      else if (item != dynamic_cast<QGraphicsRectItem *>(item))
        linkList = item->parentItem()->childItems();

      if (LinkItem *link = dynamic_cast<LinkItem *>(item)) {
        item->setOpacity(1);
        if (!cursorSet) {
          QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
          cursorSet = true;
        }
      } else if (cursorSet) {
        QApplication::restoreOverrideCursor();
        cursorSet = false;
      }
      if (AnnotZone *annotZone = dynamic_cast<AnnotZone *>(item)) {
        if (annotZone->hasText() or annotZone->hasAuthor())
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
    if (LinkItem *link = dynamic_cast<LinkItem *>(item)) {
      BACKEND->handleLink(this, link->getData()->text());
      link->setOpacity(0.1);
    }
  }

  void showEvent(QShowEvent *e) Q_DECL_OVERRIDE {
    QGraphicsView::showEvent(e);
    emit resized();
  }
};
#endif
