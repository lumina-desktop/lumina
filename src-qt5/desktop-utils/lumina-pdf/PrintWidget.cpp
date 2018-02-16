#include "PrintWidget.h"

PrintWidget::PrintWidget(QWidget *parent) : QGraphicsView(parent), scene(0), curPage(1),
  viewMode(SinglePageView), zoomMode(FitInView), zoomFactor(1), initialized(false), fitting(true) {

  this->setMouseTracking(true);
  QList<QWidget*> children = this->findChildren<QWidget*>("",Qt::FindChildrenRecursively);
  for(int i=0; i<children.length(); i++){
    children[i]->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(children[i], SIGNAL(customContextMenuRequested(const QPoint&)), this, SIGNAL(customContextMenuRequested(const QPoint&)) );
  }
	this->setInteractive(false);
	this->setDragMode(QGraphicsView::ScrollHandDrag);
	this->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
  this->setFocusPolicy(Qt::NoFocus);
	QObject::connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)),
									 this, SLOT(updateCurrentPage()));
	QObject::connect(this, SIGNAL(resized()), this, SLOT(fit()));

	scene = new QGraphicsScene(this);
	scene->setBackgroundBrush(Qt::gray);
	this->setScene(scene);
  this->degrees = 0;
  this->rotMatrix = QMatrix(1, 0, 0, 1, 0 ,0); 

	/*QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(this);*/
}

PrintWidget::~PrintWidget() {
  //delete scene;
  //delete items in pages(?)
}

//Public Slots

void PrintWidget::fitView() {
  setZoomMode(FitInView);
  setCurrentPage(publicPageNum); //Make sure we stay on the same page
}

void PrintWidget::fitToWidth() {
  setZoomMode(FitToWidth);
  setCurrentPage(publicPageNum); //Make sure we stay on the same page
}

void PrintWidget::setZoomMode(ZoomMode mode) {
  zoomMode = mode;
	fitting = true;
	fit(true);
}

void PrintWidget::setAllPagesViewMode() {
  setViewMode(AllPagesView);
}

void PrintWidget::setSinglePageViewMode() {
  setViewMode(SinglePageView);
}

void PrintWidget::setFacingPagesViewMode() {
  setViewMode(FacingPagesView);
}

void PrintWidget::setViewMode(ViewMode mode) {
	viewMode = mode;
	layoutPages();
	if (viewMode == AllPagesView) {
		this->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
		fitting = false;
		zoomMode = CustomZoom;
		//zoomFactor = this->transform().m11() * (double(printer->logicalDpiY()) / logicalDpiY());
	} else {
		fitting = true;
		fit();
	}
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
	this->scale(1/factor, 1/factor);
}

void PrintWidget::updatePreview() {
  initialized = true;
  generatePreview();
  this->updateGeometry();
}

void PrintWidget::setVisible(bool visible) {
	if(visible and !initialized)
		updatePreview();
	QGraphicsView::setVisible(visible);
}

void PrintWidget::setCurrentPage(int pageNumber) {
	if(pageNumber < 0 || pageNumber > (pages.count()+1) ){ return; }
	publicPageNum = pageNumber; //publicly requested page number (+/- 1 from actual page range)
	emit currentPageChanged();
	if(pageNumber < 1 || pageNumber > pages.count())
		return;
	int lastPage = curPage;
	curPage = pageNumber;

	if (lastPage != curPage && lastPage > 0 && lastPage <= pages.count()) {
		if (zoomMode != FitInView) {
			QScrollBar *hsc = this->horizontalScrollBar();
			QScrollBar *vsc = this->verticalScrollBar();
			QPointF pt = this->transform().map(pages.at(curPage-1)->pos());
			vsc->setValue(int(pt.y()) - 10);
			hsc->setValue(int(pt.x()) - 10);
		} else {
			this->centerOn(pages.at(curPage-1));
		}
	}
}

void PrintWidget::highlightText(int pageNum, fz_rect &rect) {
  //PageItem *item = static_cast<PageItem*>(pages[pageNum]);
  QPainter painter(this);
  painter.fillRect(QRectF(QPointF(rect.x0, rect.y0), QPointF(rect.x1, rect.y1)), QColor(255, 255, 177, 128));
}

//Private functions

void PrintWidget::generatePreview() {
	populateScene(); // i.e. setPreviewPrintedPictures() e.l.
	layoutPages();
	curPage = qBound(1, curPage, pages.count());
	publicPageNum = curPage;
	emit currentPageChanged();
	if (fitting){ fit(); }
}

void PrintWidget::layoutPages() {
	int numPages = pages.count();
	if (numPages < 1)
		return;

	int numPagePlaces = numPages;
	int cols = 1; // singleMode and default
	if (viewMode == AllPagesView) {
    cols = ((pictures->value(0)).width() > (pictures->value(0)).height()) ? qFloor(qSqrt(numPages)) : qCeil(qSqrt(numPages));
    cols += cols % 2;  // Nicer with an even number of cols
  } else if (viewMode == FacingPagesView) {
    cols = 2;
    numPagePlaces += 1;
  }
  int rows = qCeil(double(numPagePlaces) / cols);

  double itemWidth = pages.at(0)->boundingRect().width();
  double itemHeight = pages.at(0)->boundingRect().height();
  int pageNum = 1; for (int i = 0; i < rows && pageNum <= numPages; i++) {
    for (int j = 0; j < cols && pageNum <= numPages; j++) {
      if (!i && !j && viewMode == FacingPagesView) {
          continue;
      } else {
        pages.at(pageNum-1)->setPos(QPointF(j*itemWidth, i*itemHeight));
        pageNum++;
      }
		}
	}
	scene->setSceneRect(scene->itemsBoundingRect());
}

void PrintWidget::populateScene()
{
	for (int i = 0; i < pages.size(); i++)
		scene->removeItem(pages.at(i));
	qDeleteAll(pages);
	pages.clear();

	int numPages = pictures->count();
  //Replace from loadingHash resolution
	QSize paperSize = pictures->value(0).size();
  //qDebug() << "Image paperSize" << paperSize;

  //Changes the paper orientation if rotated by 90 or 270 degrees
  if(degrees == 90 or degrees == 270) 
    paperSize.transpose();

	for (int i = 0; i < numPages; i++) {
    QImage pagePicture = pictures->value(i);
    if(degrees != 0) {
      pagePicture = pagePicture.transformed(rotMatrix, Qt::SmoothTransformation);
      qDebug() << "Rotating by: " << degrees << " degrees";
    }
    if(pagePicture.isNull()) {
      qDebug() << "NULL IMAGE ON PAGE " << i;
      continue;
    }
		PageItem* item = new PageItem(i+1, pagePicture, paperSize);
		scene->addItem(item);
		pages.append(item);
	}
}

//Private Slots
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
	int newPage = curPage;
	QRect viewRect = this->viewport()->rect();
	QList<QGraphicsItem*> items = this->items(viewRect);
	for (int i=0; i<items.size(); ++i) {
		PageItem* pg = static_cast<PageItem*>(items.at(i));
		QRect overlap = this->mapFromScene(pg->sceneBoundingRect()).boundingRect() & viewRect;
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
			QList<QGraphicsItem*> containedItems = this->items(viewRect, Qt::ContainsItemBoundingRect);
			foreach(QGraphicsItem* item, containedItems) {
				PageItem* pg = static_cast<PageItem*>(item);
				if (pg->pageNumber() == curPage)
					return;
			}
		}

		int newPage = calcCurrentPage();
		if (newPage != curPage)
			curPage = newPage;
	}

	QRectF target = pages.at(curPage-1)->sceneBoundingRect();
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
			QRectF viewSceneRect = this->viewportTransform().mapRect(this->viewport()->rect());
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

	//zoomFactor = this->transform().m11() * (float(printer->logicalDpiY()) / this->logicalDpiY());
}

void PrintWidget::setPictures(QHash<int, QImage> *hash) {
  pictures = hash;
}

void PrintWidget::receiveDocument(fz_document *DOC) {
  this->doc = DOC;
  this->setVisible(true);
}

//Sets how much to rotate the image, by either 90, 180, or 270 degrees. Adds 90 degrees for cw and -90 for ccw.
void PrintWidget::setDegrees(int degrees) {
  //Mods by 360, but adds and remods because of how C++ treats negative mods
  this->degrees = ( ( ( this->degrees + degrees ) % 360 ) + 360 ) % 360;
  switch(this->degrees) {
    case 270:
      rotMatrix = QMatrix(0, -1, 1, 0, 0, 0); 
      break;
    case 90:
      rotMatrix = QMatrix(0, 1, -1, 0, 0, 0);
      break;
    case 180:
      rotMatrix = QMatrix(-1, 0, 0, -1, 0, 0);
      break;
    default:
      rotMatrix = QMatrix(1, 0, 0, 1, 0 ,0); 
  }
  this->updatePreview();
}
