#ifndef textData_H
#define textData_H

#include <QRect>

class TextData {
private:
  QRectF p_loc;
  bool p_highlighted = false;
  int p_page = 0;
  QString p_text = "";
  // int p_degrees=0;

public:
  TextData(QRectF _loc, int _page, QString _text)
      : p_loc(_loc), p_page(_page), p_text(_text)
  // p_degrees(_degrees)
  {}

  QRectF loc() { return p_loc; }
  bool highlighted() { return p_highlighted; }
  int page() { return p_page; }
  QString text() { return p_text; }
  // int degrees() { return p_degrees; }

  void loc(QRect loc) { p_loc = loc; }
  void highlighted(bool highlighted) { p_highlighted = highlighted; }
  void page(int page) { p_page = page; }
  void text(QString text) { p_text = text; }
  // void degrees(int degrees) { p_degrees = degrees; }
};

#endif
