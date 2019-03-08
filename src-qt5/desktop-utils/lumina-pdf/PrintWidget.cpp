#include "PrintWidget.h"
#include <QGraphicsProxyWidget>
#include <QPushButton>
#include <QtConcurrent>

PrintWidget::PrintWidget(Renderer *backend, QWidget *parent)
    : QGraphicsView(parent), scene(0), curPage(1), viewMode(SinglePageView),
      zoomMode(FitInView), zoomFactor(1), initialized(false), fitting(true),
      BACKEND(backend) {

  this->setMouseTracking(true);
  QList<QWidget *> children =
      this->findChildren<QWidget *>("", Qt::FindChildrenRecursively);
  for (int i = 0; i < children.length(); i++) {
    children[i]->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(children[i],
                     SIGNAL(customContextMenuRequested(const QPoint &)), this,
                     SIGNAL(customContextMenuRequested(const QPoint &)));
  }
  this->setInteractive(false);
  this->setDragMode(QGraphicsView::ScrollHandDrag);
  this->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
  this->setFocusPolicy(Qt::NoFocus);
  QObject::connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), this,
                   SLOT(updateCurrentPage()));
  QObject::connect(this, SIGNAL(resized()), this, SLOT(fit()));

  scene = new QGraphicsScene(this);
  scene->setBackgroundBrush(Qt::gray);
  this->setScene(scene);

  /*QVBoxLayout *layout = new QVBoxLayout;
  setLayout(layout);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(this);*/
}

PrintWidget::~PrintWidget() {
  for (int i = 0; i < pages.size(); i++) {
    scene->removeItem(pages.at(i));
  }
  qDeleteAll(pages);
  pages.clear();
  scene->deleteLater();
}

// Public Slots

void PrintWidget::fitView() {
  setZoomMode(FitInView);
  setCurrentPage(curPage); // Make sure we stay on the same page
  //generatePreview();
}

void PrintWidget::fitToWidth() {
  setZoomMode(FitToWidth);
  setCurrentPage(curPage); // Make sure we stay on the same page
 // generatePreview();
}

void PrintWidget::setZoomMode(ZoomMode mode) {
  zoomMode = mode;
  fitting = true;
  fit(true);
  generatePreview();
}

void PrintWidget::setAllPagesViewMode() { setViewMode(AllPagesView); }

void PrintWidget::setSinglePageViewMode() { setViewMode(SinglePageView); }

void PrintWidget::setFacingPagesViewMode() { setViewMode(FacingPagesView); }

void PrintWidget::setViewMode(ViewMode mode) {
  viewMode = mode;
  layoutPages();
  if (viewMode == AllPagesView) {
    this->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    fitting = false;
    zoomMode = CustomZoom;
    zoomFactor = this->transform().m11() * (150.0 / logicalDpiY());
  } else {
    fitting = true;
    fit();
  }
  generatePreview();
}

void PrintWidget::zoomIn(double factor) {
  fitting = false;
  zoomMode = CustomZoom;
  zoomFactor *= factor;
  this->scale(factor, factor);
}

void PrintWidget::zoomOut(double factor) {
  fitting = false;
  zoomMode = CustomZoom;
  zoomFactor *= factor;
  this->scale(1 / factor, 1 / factor);
}

void PrintWidget::updatePreview() {
  initialized = true;
  generatePreview();
  this->updateGeometry();
}

void PrintWidget::setVisible(bool visible) {
  if (visible and !initialized)
    updatePreview();
  QGraphicsView::setVisible(visible);
}

void PrintWidget::setCurrentPage(int pageNumber) {

  int lastPage = curPage;
  //We need to allow one additional page after the last page here for presentation mode.
  // This allows a "blank" page at the end of a presentation before actually closing down the presentation;
  curPage = std::max(1, std::min(pageNumber, BACKEND->numPages()+1));

   /*if (lastPage != curPage && lastPage > 0 && lastPage <= pages.count()) {
    if (zoomMode != FitInView) {
      QScrollBar *hsc = this->horizontalScrollBar();
      QScrollBar *vsc = this->verticalScrollBar();
      QPointF pt = this->transform().map(pages.at(curPage - 1)->pos());
      vsc->setValue(int(pt.y()) - 10);
      hsc->setValue(int(pt.x()) - 10);
    } else {
      this->centerOn(pages.at(curPage - 1));
    }
  }*/

  QSize DPI(300, 300);
  if (lastPage != curPage || !BACKEND->imageSize(curPage).isNull()){
    updatePreview();
    for(int i=(curPage-3); i<=BACKEND->numPages(); i++){
      if(i<0){ continue; }
      else if( i < (curPage-2) ){ BACKEND->clearHash(i); }
      else if( i > (curPage+2) ){ BACKEND->clearHash(i); }
      else{
         if (BACKEND->loadMultiThread()) {
          QtConcurrent::run(BACKEND, &Renderer::renderPage, i, DPI, degrees);
        } else {
          BACKEND->renderPage(i, DPI, degrees);
        }
      }
    }
  }
  //qDebug() << "Current page set to " << pageNumber << "\n";
}

void PrintWidget::highlightText(TextData *text) {
  // Creates a rectangle around the text if the text has not already been
  // highlighted qDebug() << "Page:" << text->page() << "Loc:" << text->loc();
  if (!text->highlighted() && !text->loc().isNull()) {
    int degrees = BACKEND->rotatedDegrees();
    // qDebug() << "Degrees:" << degrees;
    // Shows the text's location on a non-rotated page
    QRectF rect = text->loc();
    // Rotates the rectangle by the page's center and gets the right calculation
    // for text's new location
    if (degrees != 0) {
      QSize center = BACKEND->imageSize(text->page() - 1) / 2;

      if (degrees == 90 or degrees == 270)
        center.transpose();

      double cx = center.width(), cy = center.height();
      rect.adjust(-cx, -cy, -cx, -cy);
      QMatrix matrix;
      matrix.rotate(BACKEND->rotatedDegrees());
      rect = matrix.mapRect(rect);
      if (BACKEND->rotatedDegrees() == 180)
        rect.adjust(cx, cy, cx, cy);
      else
        rect.adjust(cy, cx, cy, cx);
    }

    // qDebug() << "Post Degrees:" << rect;
    // Moves the rectangle onto the right page
    double pageHeight = 0;
    for (int i = 0; i < text->page() - 1; i++)
      pageHeight += pages.at(i)->boundingRect().height();

    // qDebug() << "PageHeight:" << pageHeight;

    rect.moveTop(rect.y() + pageHeight);

    // qDebug() << "Final Rect:" << rect;
    // Transparent yellow for the highlight box
    QBrush highlightFill(QColor(255, 255, 177, 100));
    QPen highlightOutline(QColor(255, 255, 100, 125));
    scene->addRect(rect, highlightOutline, highlightFill);
    text->highlighted(true);
  }
  goToPosition(text->page(), text->loc().x(), text->loc().y());
}

// Private functions

void PrintWidget::generatePreview() {
  populateScene(); // i.e. setPreviewPrintedPictures() e.l.
  layoutPages();
  // curPage = qBound(1, curPage, pages.count());
  publicPageNum = curPage;
  emit currentPageChanged();
  if (fitting) {
    fit();
  }
}

void PrintWidget::layoutPages() {
  int numPages = pages.count();
  if (numPages < 1)
    return;

  int numPagePlaces = numPages;
  int cols = 1; // singleMode and default
  QSize pageSize = BACKEND->imageSize(curPage);
  if (viewMode == AllPagesView) {
    cols = pageSize.width() > pageSize.height() ? qFloor(qSqrt(numPages))
                                                : qCeil(qSqrt(numPages));
    cols += cols % 2; // Nicer with an even number of cols
  } else if (viewMode == FacingPagesView) {
    cols = 2;
    numPagePlaces += 1;
  }
  int rows = qCeil(double(numPagePlaces) / cols);

  int pageNum = 0;
  QList<double> rowMaxList;
  for (int i = 0; i < rows && pageNum < numPages; i++) {
    double rowMax = 0;
    for (int j = 0; j < cols && pageNum < numPages; j++) {
      double itemWidth = 0, itemHeight = 0;
      double pageHeight = pages.at(pageNum)->boundingRect().height();

      for (int k = cols * (pageNum / cols); k < pageNum; k++)
        itemWidth += pages.at(k)->boundingRect().width();

      foreach (double size, rowMaxList)
        itemHeight += size;

      pages.at(pageNum)->setPos(QPointF(itemWidth, itemHeight));
      pageNum++;
      rowMax = qMax(rowMax, pageHeight);
    }
    rowMaxList.push_back(rowMax);
  }
  scene->setSceneRect(scene->itemsBoundingRect());
  // qDebug() << "Finished Page Layout";
}

void PrintWidget::populateScene() {
  // qDeleteAll(scene->items().begin(), scene->items().end());
  scene->clear();
  // qDeleteAll(pages.begin(), pages.end());
  pages.clear();
  links.clear();
  annots.clear();
  // int numPages = BACKEND->numPages();
  if (!BACKEND->isDoneLoading(curPage)) {
    //qDebug() << "populateScene() called while backend still loading.\n";
    return;
  } // nothing to show yet
  int start = curPage;
  int end = start;
  if( viewMode == FacingPagesView){ end += 1; }
  else if( viewMode == AllPagesView ) {  start = 1; end = BACKEND->numPages(); }
  //single-page view otherwise
  //qDebug() << "Populate Scene" << start << end;
  for (int i = start; i < end + 1; i++) {
    QImage pagePicture = BACKEND->imageHash(i);
    QSize paperSize = pagePicture.size();
    QList<QGraphicsItem *> linkLocations;
    QList<QGraphicsItem *> annotLocations;

    if (pagePicture.isNull()) {
      //qDebug() << "NULL IMAGE ON PAGE " << i;
      continue;
    }

    PageItem *item = new PageItem(i + 1, pagePicture, paperSize, BACKEND);
    scene->addItem(item);
    pages.append(item);

    if (BACKEND->supportsExtraFeatures()) {
      for (int k = 0; k < BACKEND->linkSize(i); k++) {
        LinkItem *lItem = new LinkItem(item, BACKEND->linkList(i, k));
        lItem->setOpacity(0.1);
        linkLocations.append(lItem);
      }
      // qDebug() << "Creating annotations for:" <<  i;
      for (int k = 0; k < BACKEND->annotSize(i); k++) {
        Annotation *annot = BACKEND->annotList(i, k);
        if (annot->getType() == 14) {
          InkItem *iItem = new InkItem(item, annot);
          annotLocations.append(iItem);
        }
        PopupItem *aItem = new PopupItem(item, annot);
        AnnotZone *aZone = new AnnotZone(item, annot, aItem);
        aItem->setVisible(false);
        annotLocations.append(aItem);
        annotLocations.append(aZone);
      }

      for (int k = 0; k < BACKEND->widgetSize(i); k++) {
        Widget *widget = BACKEND->widgetList(i, k);
        int type = widget->getWidgetType();
        QRectF loc = widget->getLocation();
        QString text = widget->getCurrentText();
        switch (type) {
        case 0: {
          QPushButton *button = new QPushButton(widget->getCurrentText());
          button->setGeometry(loc.toRect());
          button->setText(text);
          QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(item);
          proxy->setWidget(button);
        } break;
        case 1: {

        } break;
        case 2: {

        } break;
        case 3: {

        } break;
        case 4: {

        } break;
        case 5: {

        } break;
        case 6: {

        } break;
        default: { qDebug() << "INVALID WIDGET"; }
        }
      }
      links.insert(i, linkLocations);
      annots.insert(i, annotLocations);
    }
  }
}

// Private Slots
void PrintWidget::updateCurrentPage() {
  if (viewMode == AllPagesView)
    return;

  int newPage = calcCurrentPage();
  if (newPage != curPage) {
    curPage = newPage;
    publicPageNum = curPage;
    emit currentPageChanged();
  }
}

int PrintWidget::calcCurrentPage() {
  int maxArea = 0;

  return curPage;

  int newPage = curPage;
  QRect viewRect = this->viewport()->rect();
  QList<QGraphicsItem *> items = this->items(viewRect);
  for (int i = 0; i < items.size(); ++i) {
    PageItem *pg = static_cast<PageItem *>(items.at(i));
    QRect overlap =
        this->mapFromScene(pg->sceneBoundingRect()).boundingRect() & viewRect;
    int area = overlap.width() * overlap.height();
    if (area > maxArea) {
      maxArea = area;
      newPage = pg->pageNumber();
    } else if (area == maxArea && pg->pageNumber() < newPage) {
      newPage = pg->pageNumber();
    }
  }
  return newPage;
}

void PrintWidget::fit(bool doFitting) {
  if (curPage < 1 || curPage > pages.count())
    return;
  if (!doFitting && !fitting)
    return;

  if (doFitting && fitting) {
    QRect viewRect = this->viewport()->rect();
    if (zoomMode == FitInView) {
      QList<QGraphicsItem *> containedItems =
          this->items(viewRect, Qt::ContainsItemBoundingRect);
      foreach (QGraphicsItem *item, containedItems) {
        PageItem *pg = static_cast<PageItem *>(item);
        if (pg->pageNumber() == curPage)
          return;
      }
    }

    int newPage = calcCurrentPage();
    if (newPage != curPage)
      curPage = newPage;
  }

  QRectF target = pages.at(curPage - 1)->sceneBoundingRect();
  if (viewMode == FacingPagesView) {
    if (curPage % 2)
      target.setLeft(target.left() - target.width());
    else
      target.setRight(target.right() + target.width());
  } else if (viewMode == AllPagesView) {
    target = scene->itemsBoundingRect();
  }

  if (zoomMode == FitToWidth) {
    QTransform t;
    qreal scale = this->viewport()->width() / target.width();
    t.scale(scale, scale);
    this->setTransform(t);
    if (doFitting && fitting) {
      QRectF viewSceneRect =
          this->viewportTransform().mapRect(this->viewport()->rect());
      viewSceneRect.moveTop(target.top());
      this->ensureVisible(viewSceneRect); // Nah...
    }
  } else {
    this->fitInView(target, Qt::KeepAspectRatio);
    if (zoomMode == FitInView) {
      int step = qRound(this->matrix().mapRect(target).height());
      this->verticalScrollBar()->setSingleStep(step);
      this->verticalScrollBar()->setPageStep(step);
    }
  }

  zoomFactor = this->transform().m11() * (150.0 / this->logicalDpiY());
}

void PrintWidget::goToPosition(int pagenum, float x, float y) {
  // qDebug() << "Page:" << pagenum << "X:" << x << "Y:" << y;
  setCurrentPage(pagenum);

  QScrollBar *hsc = this->horizontalScrollBar();
  QScrollBar *vsc = this->verticalScrollBar();
  QPointF pt = this->transform().map(pages.at(pagenum - 1)->pos());
  int secondPagenum = pagenum < pages.size() ? pagenum : pagenum - 2;
  QPointF pt2 = this->transform().map(pages.at(secondPagenum)->pos());
  double realHeight = pages.at(pagenum - 1)->boundingRect().height();
  double virtualHeight = qAbs(pt2.y() - pt.y());

  // qDebug() << "Real:" << realHeight << "Virtual:" << virtualHeight;

  int yConv = int(pt.y() + y * (virtualHeight / realHeight)) - 30;
  int xConv = int(pt.x() + x * (virtualHeight / realHeight)) - 30;

  // qDebug() << "newX:" << xConv << "newY:" << yConv;

  if (yConv > vsc->maximum())
    vsc->triggerAction(QAbstractSlider::SliderToMaximum);
  else if (y != 0)
    vsc->setValue(yConv);

  if (x != 0)
    hsc->setValue(xConv);
}
