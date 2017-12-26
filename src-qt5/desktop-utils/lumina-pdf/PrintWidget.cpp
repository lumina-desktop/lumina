#include "PrintWidget.h"

class PageItem : public QGraphicsItem
{
public:
    PageItem(int _pageNum, const QPicture* _pagePicture, QSize _paperSize, QRect _pageRect)
        : pageNum(_pageNum), pagePicture(_pagePicture),
          paperSize(_paperSize), pageRect(_pageRect)
    {
        qreal border = qMax(paperSize.height(), paperSize.width()) / 25;
        brect = QRectF(QPointF(-border, -border),
                       QSizeF(paperSize)+QSizeF(2*border, 2*border));
        setCacheMode(DeviceCoordinateCache);
    }

    QRectF boundingRect() const Q_DECL_OVERRIDE
    { return brect; }

    inline int pageNumber() const
    { return pageNum; }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) Q_DECL_OVERRIDE;

private:
    int pageNum;
    const QPicture* pagePicture;
    QSize paperSize;
    QRect pageRect;
    QRectF brect;
};

void PageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

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
    if (!pagePicture)
        return;
    painter->drawPicture(pageRect.topLeft(), *pagePicture);

    // Effect: make anything drawn in the margins look washed out.
    QPainterPath path;
    path.addRect(paperRect);
    path.addRect(pageRect);
    painter->setPen(QPen(Qt::NoPen));
    painter->setBrush(QColor(255, 255, 255, 180));
    painter->drawPath(path);
}
PrintWidget::PrintWidget(QPrinter *printer, QWidget *parent) : QWidget(parent), scene(0), curPage(1), viewMode(SinglePageView), zoomMode(FitInView), zoomFactor(1), initialized(false), fitting(true) {
	this->printer = printer;
  this->setMouseTracking(true);
  QList<QWidget*> children = this->findChildren<QWidget*>("",Qt::FindChildrenRecursively);
  for(int i=0; i<children.length(); i++){
    children[i]->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(children[i], SIGNAL(customContextMenuRequested(const QPoint&)), this, SIGNAL(customContextMenuRequested(const QPoint&)) );
  }
	graphicsView = new GraphicsView;
	graphicsView->setInteractive(false);
	graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
	graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
	QObject::connect(graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)),
									 this, SLOT(updateCurrentPage()));
	QObject::connect(graphicsView, SIGNAL(resized()), this, SLOT(fit()));

	scene = new QGraphicsScene(graphicsView);
	scene->setBackgroundBrush(Qt::gray);
	graphicsView->setScene(scene);

	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(graphicsView);
}

PrintWidget::~PrintWidget() {
	//Nothing here for now
}

//Public Slots

void PrintWidget::fitInView() {
  setZoomMode(FitToWidth);
}

void PrintWidget::fitToWidget() {
  setZoomMode(FitInView);
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
		graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
		fitting = false;
		zoomMode = CustomZoom;
		zoomFactor = graphicsView->transform().m11() * (double(printer->logicalDpiY()) / logicalDpiY());
		emit previewChanged();
	} else {
		fitting = true;
		fit();
	}
}

void PrintWidget::zoomIn(double factor) {
	fitting = false;
	zoomMode = CustomZoom;
	zoomFactor *= factor;
	graphicsView->scale(factor, factor);
}

void PrintWidget::zoomOut(double factor) {
	fitting = false;
	zoomMode = CustomZoom;
	zoomFactor *= factor;
	graphicsView->scale(1/factor, 1/factor);
}

void PrintWidget::updatePreview() {
  initialized = true;
  generatePreview();
  graphicsView->updateGeometry();
}

void PrintWidget::setOrientation(QPrinter::Orientation orientation) {
	printer->setOrientation(orientation);
	generatePreview();
}

void PrintWidget::setVisible(bool visible) {
	if(visible and !initialized)
		updatePreview();
	QWidget::setVisible(visible);
}

void PrintWidget::setCurrentPage(int pageNumber) {
	if(pageNumber < 1 || pageNumber > pages.count())
		return;

	int lastPage = curPage;
	curPage = pageNumber;

	if (lastPage != curPage && lastPage > 0 && lastPage <= pages.count()) {
		if (zoomMode != FitInView) {
			QScrollBar *hsc = graphicsView->horizontalScrollBar();
			QScrollBar *vsc = graphicsView->verticalScrollBar();
			QPointF pt = graphicsView->transform().map(pages.at(curPage-1)->pos());
			vsc->setValue(int(pt.y()) - 10);
			hsc->setValue(int(pt.x()) - 10);
		} else {
			graphicsView->centerOn(pages.at(curPage-1));
		}
	}
}

//Private functions

void PrintWidget::generatePreview() {
	//printer->d_func()->setPreviewMode(true);
	emit paintRequested(printer);
	//printer->d_func()->setPreviewMode(false);
	//pictures = printer->previewPages();
	populateScene(); // i.e. setPreviewPrintedPictures() e.l.
	layoutPages();
	curPage = qBound(1, curPage, pages.count());
	if (fitting)
			fit();
	emit previewChanged();	
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
	int pageNum = 1;
	for (int i = 0; i < rows && pageNum <= numPages; i++) {
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

	int numPages = pictures.count();
	QSize paperSize = printer->pageLayout().fullRectPixels(printer->resolution()).size();
	QRect pageRect = printer->pageLayout().paintRectPixels(printer->resolution());

	for (int i = 0; i < numPages; i++) {
		PageItem* item = new PageItem(i+1, pictures.at(i), paperSize, pageRect);
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
		emit previewChanged();
	}
}

int PrintWidget::calcCurrentPage() {
	int maxArea = 0;
	int newPage = curPage;
	QRect viewRect = graphicsView->viewport()->rect();
	QList<QGraphicsItem*> items = graphicsView->items(viewRect);
	for (int i=0; i<items.size(); ++i) {
		PageItem* pg = static_cast<PageItem*>(items.at(i));
		QRect overlap = graphicsView->mapFromScene(pg->sceneBoundingRect()).boundingRect() & viewRect;
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
		QRect viewRect = graphicsView->viewport()->rect();
		if (zoomMode == FitInView) {
			QList<QGraphicsItem*> containedItems = graphicsView->items(viewRect, Qt::ContainsItemBoundingRect);
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
		qreal scale = graphicsView->viewport()->width() / target.width();
		t.scale(scale, scale);
		graphicsView->setTransform(t);
		if (doFitting && fitting) {
			QRectF viewSceneRect = graphicsView->viewportTransform().mapRect(graphicsView->viewport()->rect());
			viewSceneRect.moveTop(target.top());
			graphicsView->ensureVisible(viewSceneRect); // Nah...
		}
	} else {
		graphicsView->fitInView(target, Qt::KeepAspectRatio);
		if (zoomMode == FitInView) {
			int step = qRound(graphicsView->matrix().mapRect(target).height());
			graphicsView->verticalScrollBar()->setSingleStep(step);
			graphicsView->verticalScrollBar()->setPageStep(step);
		}
	}

	zoomFactor = graphicsView->transform().m11() * (float(printer->logicalDpiY()) / this->logicalDpiY());
	emit previewChanged();
}
