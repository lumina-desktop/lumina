#ifndef LUMINA_PDF_GRAPHICSITEMS_H
#define LUMINA_PDF_GRAPHICSITEMS_H

#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOption>
#include <QTextDocument>

class InkItem : public QGraphicsItem {
public:
  InkItem(QGraphicsItem *parent, Annotation *_annot)
      : QGraphicsItem(parent), pointData(_annot->getInkList()),
        inkColor(_annot->getColor()), annot(_annot) {
    setCacheMode(DeviceCoordinateCache);
    bbox = annot->getLoc();
  }

  QRectF boundingRect() const Q_DECL_OVERRIDE { return bbox; }

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) Q_DECL_OVERRIDE {
    Q_UNUSED(widget);
    painter->setClipRect(option->exposedRect);
    QPen inkPen = QPen(inkColor);
    painter->setPen(inkPen);
    foreach (QVector<QPointF> pointList, pointData) {
      painter->drawLines(pointList);
    }
  }

private:
  QRectF bbox;
  QVector<QVector<QPointF>> pointData;
  QColor inkColor;
  Annotation *annot;
};

class PopupItem : public QGraphicsItem {
public:
  PopupItem(QGraphicsItem *parent, Annotation *_annot)
      : QGraphicsItem(parent), author(_annot->getAuthor()),
        text(_annot->getText()) {
    QRectF loc = _annot->getLoc();
    setCacheMode(DeviceCoordinateCache);
    QString allText = "Author: " + author + "\n\n" + text;
    QTextDocument document;
    document.setDefaultFont(QFont("Helvitica", 10, QFont::Normal));
    document.setPageSize(QSize(120, 120));
    document.setHtml(allText);
    loc.moveTopLeft(
        QPointF(loc.center().x(), loc.center().y() + loc.height() / 2));
    loc.setSize(document.size() + QSize(10, 10));
    bbox = loc;
  }

  QRectF boundingRect() const Q_DECL_OVERRIDE { return bbox; }

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) Q_DECL_OVERRIDE {
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

class AnnotZone : public QGraphicsItem {
public:
  AnnotZone(QGraphicsItem *parent, Annotation *_annot, PopupItem *_annotItem)
      : QGraphicsItem(parent), bbox(_annot->getLoc()), annot(_annotItem) {
    _hasText = !_annot->getText().isEmpty();
    _hasAuthor = !_annot->getAuthor().isEmpty();
  }

  QRectF boundingRect() const Q_DECL_OVERRIDE { return bbox; }
  PopupItem *annotation() const { return annot; }
  bool hasText() const { return _hasText; }
  bool hasAuthor() const { return _hasAuthor; }
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) Q_DECL_OVERRIDE {
    Q_UNUSED(widget);
    Q_UNUSED(painter);
    Q_UNUSED(option);
  }

private:
  QRectF bbox;
  PopupItem *annot;
  bool _hasText;
  bool _hasAuthor;
};

class LinkItem : public QGraphicsItem {
public:
  LinkItem(QGraphicsItem *parent, TextData *_data)
      : QGraphicsItem(parent), bbox(_data->loc()), data(_data) {
    setCacheMode(DeviceCoordinateCache);
    setAcceptHoverEvents(true);
  }

  QRectF boundingRect() const Q_DECL_OVERRIDE { return bbox; }

  inline TextData *getData() const { return data; }

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) Q_DECL_OVERRIDE {
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
  PageItem(int _pageNum, QImage _pagePicture, QSize _paperSize,
           Renderer *_backend)
      : pageNum(_pageNum), pagePicture(_pagePicture), paperSize(_paperSize),
        BACKEND(_backend) {
    brect = QRectF(QPointF(-25, -25), QSizeF(paperSize) + QSizeF(50, 50));
    setCacheMode(DeviceCoordinateCache);
    setAcceptHoverEvents(true);
  }

  QRectF boundingRect() const Q_DECL_OVERRIDE { return brect; }

  inline int pageNumber() const { return pageNum; }

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) Q_DECL_OVERRIDE {
    Q_UNUSED(widget);
    // Ensure all the antialiasing/smoothing options are turned on
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    QRectF paperRect(0, 0, paperSize.width(), paperSize.height());
    // Draw shadow
    painter->setClipRect(option->exposedRect);
    qreal shWidth = paperRect.width() / 100;
    QRectF rshadow(paperRect.topRight() + QPointF(0, shWidth),
                   paperRect.bottomRight() + QPointF(shWidth, 0));
    QLinearGradient rgrad(rshadow.topLeft(), rshadow.topRight());
    rgrad.setColorAt(0.0, QColor(0, 0, 0, 255));
    rgrad.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter->fillRect(rshadow, QBrush(rgrad));
    QRectF bshadow(paperRect.bottomLeft() + QPointF(shWidth, 0),
                   paperRect.bottomRight() + QPointF(0, shWidth));
    QLinearGradient bgrad(bshadow.topLeft(), bshadow.bottomLeft());
    bgrad.setColorAt(0.0, QColor(0, 0, 0, 255));
    bgrad.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter->fillRect(bshadow, QBrush(bgrad));
    QRectF cshadow(paperRect.bottomRight(),
                   paperRect.bottomRight() + QPointF(shWidth, shWidth));
    QRadialGradient cgrad(cshadow.topLeft(), shWidth, cshadow.topLeft());
    cgrad.setColorAt(0.0, QColor(0, 0, 0, 255));
    cgrad.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter->fillRect(cshadow, QBrush(cgrad));
    painter->setClipRect(paperRect & option->exposedRect);
    painter->fillRect(paperRect, Qt::white);
    painter->drawImage(QPoint(0, 0), pagePicture);
    for (int k = 0; k < BACKEND->annotSize(pageNum - 1); k++) {
      Annotation *annot = BACKEND->annotList(pageNum - 1, k);
      painter->drawImage(annot->getLoc(), annot->renderImage());
    }
  }

private:
  int pageNum;
  QImage pagePicture;
  QSize paperSize;
  QRectF brect;
  Renderer *BACKEND;
};

#endif // LUMINA_PDF_GRAPHICSITEMS_H
