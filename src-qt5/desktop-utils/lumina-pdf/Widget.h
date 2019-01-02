#include <QStringList>

class Widget {
public:
  Widget(int _widgetType, QRectF _loc, QString _currText, int _maxLen,
         int _contentType)
      : widgetType(_widgetType), loc(_loc), currText(_currText),
        maxLen(_maxLen), contentType(_contentType) {}

  virtual ~Widget() {}

  virtual int getWidgetType() { return widgetType; }
  virtual QRectF getLocation() { return loc; }
  virtual QString getCurrentText() { return currText; }
  virtual int getMaxLength() { return maxLen; }
  virtual int getContentType() { return contentType; }
  virtual QStringList getOptionList() { return optionList; }
  virtual QStringList getExportList() { return exportList; }

  virtual void setOptions(QStringList _optionList) { optionList = _optionList; }
  virtual void setExports(QStringList _exportList) { exportList = _exportList; }

private:
  int widgetType;
  QRectF loc;
  QString currText;
  int maxLen;
  int contentType;

  QStringList optionList;
  QStringList exportList;
};
