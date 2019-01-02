#pragma once

class Bookmark {
public:
  Bookmark(char *_title, char *_link, int _pagenum, int _level)
      : pagenum(_pagenum), level(_level) {
    title = QString::fromLocal8Bit(_title);
    link = QString::fromLocal8Bit(_link);
  }

  QString title;
  QString link;
  int pagenum;
  int level;
};
