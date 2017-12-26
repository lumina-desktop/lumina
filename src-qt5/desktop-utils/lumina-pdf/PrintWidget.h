//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// Simple subclass of QPrintPreviewWidget to provide
// notification when a context menu is requested
//===========================================
#ifndef _PRINT_PREVIEW_WIDGET_H
#define _PRINT_PREVIEW_WIDGET_H

#include <QMouseEvent>
#include <QDebug>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QBoxLayout>
#include <QScrollBar>
#include <QStyleOptionGraphicsItem>
#include <QtMath>
#include <QPrinter>

class GraphicsView : public QGraphicsView
{
	Q_OBJECT
public:
	GraphicsView(QWidget *parent = 0) : QGraphicsView(parent) { }
signals:
	void resized();
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

class PrintWidget: public QWidget{
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

	PrintWidget(QPrinter *printer = 0, QWidget *parent = 0);
  ~PrintWidget();

  int currentPage() const { return curPage; };
  double getZoomFactor() const { return this->zoomFactor; };
  QPrinter::Orientation orientation() const { return printer->orientation(); };
  ZoomMode getZoomMode() const { return this->zoomMode; };
  void setVisible(bool);

public slots:
  void zoomIn(double factor=1.2);
  void zoomOut(double factor=1.2);
  void setOrientation(QPrinter::Orientation);
  void setCurrentPage(int);

	void fitInView();
  void fitToWidget();
  void setAllPagesViewMode();
  void setSinglePageViewMode();
  void setFacingPagesViewMode();

  void updatePreview();

private slots:
  void updateCurrentPage();
  int calcCurrentPage();
  void fit(bool doFitting=false);

signals:
	void customContextMenuRequested(const QPoint&);
	void paintRequested(QPrinter*);
  void previewChanged();

private:
  void generatePreview();
  void layoutPages();
  void populateScene();
  void setViewMode(ViewMode viewMode);
  void setZoomMode(ZoomMode viewMode);

  QPrinter *printer;
  GraphicsView *graphicsView;
  QGraphicsScene *scene;
	int curPage;
	QList<const QPicture *> pictures;
	QList<QGraphicsItem *> pages;

	ViewMode viewMode;
	ZoomMode zoomMode;
	double zoomFactor;
	bool initialized, fitting;
};

#endif
