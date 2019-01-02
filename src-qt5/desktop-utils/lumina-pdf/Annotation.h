#include <QColor>
#include <QList>
#include <QPointF>
#include <QPolygonF>
#include <QVector>

class Annotation {
public:
  Annotation(int _annotType, double _opacity, QRectF _loc = QRectF())
      : annotType(_annotType), opacity(_opacity), loc(_loc) {}

  virtual ~Annotation() {}

  virtual int getType() { return annotType; }
  virtual QRectF getLoc() { return loc; }

  virtual QString getAuthor() { return author; }
  virtual QString getText() { return text; }
  virtual QList<QPolygonF> getQuadList() { return quadList; }
  virtual QVector<QVector<QPointF>> getInkList() { return inkList; }
  virtual QColor getColor() { return color; }
  virtual QColor getInternalColor() { return iColor; }
  virtual double getOpacity() { return opacity; }
  virtual bool print() { return canPrint; }

  virtual void setAuthor(QString _author) { author = _author; }
  virtual void setContents(QString _text) { text = _text; }
  virtual void setColor(QColor _color) { color = _color; };
  virtual void setInternalColor(QColor _iColor) { iColor = _iColor; };
  virtual void setQuadList(QList<QPolygonF> _quadList) {
    quadList = _quadList;
  };
  virtual void setInkList(QVector<QVector<QPointF>> _inkList) {
    inkList = _inkList;
  };
  virtual void setPrint(bool _print) { canPrint = _print; }

  virtual QImage renderImage() = 0;

private:
  int annotType;
  double opacity;
  QRectF loc;

  QString author;
  QString text;
  QColor color;
  QColor iColor;
  QList<QPolygonF> quadList;
  QVector<QVector<QPointF>> inkList;
  bool canPrint;
};
