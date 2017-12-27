#include "PrintWidget.h"

PrintWidget::PrintWidget(QPrinter* printer, QWidget *parent) : QGraphicsView(parent), scene(0), curPage(1), 
  viewMode(SinglePageView), zoomMode(FitInView), zoomFactor(1), initialized(false), fitting(true) {

  this->printer = printer;
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
}

void PrintWidget::fitToWidth() {
  setZoomMode(FitToWidth);
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
		zoomFactor = this->transform().m11() * (double(printer->logicalDpiY()) / logicalDpiY());
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

//Private functions

void PrintWidget::generatePreview() {
  qDebug() << "generating preview";
  if(!previewEngine)
    previewEngine = new QPreviewPaintEngine();
  if(!pdfEngine) 
    pdfEngine = new QPdfPrintEngine(QPrinter::HighResolution);

  printer->setEngines(previewEngine, previewEngine);
  previewEngine->setProxyEngines(pdfEngine, pdfEngine);

	emit paintRequested(printer);

  printer->setEngines(pdfEngine, pdfEngine);

  qDebug() << "Populating Scene";
	populateScene(); // i.e. setPreviewPrintedPictures() e.l.
	layoutPages();
	curPage = qBound(1, curPage, pages.count());
	if (fitting)
			fit();
}

void PrintWidget::layoutPages() {
	int numPages = pages.count();
	if (numPages < 1)
		return;

	int numPagePlaces = numPages;
	int cols = 1; // singleMode and default
	if (viewMode == AllPagesView) {
		if (printer->orientation() == QPrinter::Portrait)
			cols = qCeil(qSqrt(numPages));
    else
      cols = qFloor(qSqrt(numPages)); 
    cols += cols % 2;  // Nicer with an even number of cols 
  } 
  else if (viewMode == FacingPagesView) { 
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
  qDebug() << "Pages cleared";

	int numPages = pictures->count();
	QSize paperSize = printer->pageLayout().fullRectPixels(printer->resolution()).size();
	QRect pageRect = printer->pageLayout().paintRectPixels(printer->resolution());
  qDebug() << "Fields set";

	for (int i = 0; i < numPages; i++) {
		PageItem* item = new PageItem(i+1, (*pictures)[i], paperSize, pageRect);
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

	zoomFactor = this->transform().m11() * (float(printer->logicalDpiY()) / this->logicalDpiY());
}

void PrintWidget::setPictures(QHash<int, QImage> *hash) {
  pictures = hash;
} 
