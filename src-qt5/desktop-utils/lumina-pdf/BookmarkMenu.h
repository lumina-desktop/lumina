//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#pragma once

#include <QDialog>
#include <QTextEdit>
#include "Renderer.h"

namespace Ui{
  class BookmarkMenu;
};

class BookmarkMenu : public QWidget{
  Q_OBJECT
  public:
    BookmarkMenu(Renderer *Backend, QWidget *parent=NULL);
  public slots:
    void loadBookmarks();

  private:
    Ui::BookmarkMenu *ui;
    Renderer *BACKEND;
};
