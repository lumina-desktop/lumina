//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================

#include "BookmarkMenu.h"
#include "ui_BookmarkMenu.h"
#include <LuminaXDG.h>

BookmarkMenu::BookmarkMenu(Renderer *Backend, QWidget *parent) : QWidget(parent), ui(new Ui::BookmarkMenu()), BACKEND(Backend){
  ui->setupUi(this);
  ui->closeButton->setIcon( LXDG::findIcon("dialog-close") );
  connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
  connect(ui->bookmarks, &QTreeWidget::itemClicked, this, [=](QTreeWidgetItem *item) { 
    Backend->handleLink(item->data(1, Qt::UserRole).toString()); }); 

  ui->bookmarks->setHeaderLabel("Title"); 
}

void BookmarkMenu::loadBookmarks() {
  QTreeWidgetItem *item=nullptr, *parent=nullptr;
  QList<Bookmark*> bookmarks = BACKEND->getBookmarks();

  if(ui->bookmarks->topLevelItemCount() != 0) {
    ui->bookmarks->clear();
  }

  //Modfiy for more than 2 levels
  if(bookmarks.empty()) {
    item = new QTreeWidgetItem(ui->bookmarks);
    item->setText(0, "No Bookmarks");
    item->setData(1, Qt::UserRole, "");
    item->setIcon(0, LXDG::findIcon("bookmark-remove"));
  }else{
    foreach(Bookmark *bm, bookmarks) {
      if(bm->level == 0) {
        item = new QTreeWidgetItem(ui->bookmarks);
        parent = item;
      }else{
        item = new QTreeWidgetItem(parent);
      }

      item->setText(0, bm->title);
      item->setData(1, Qt::UserRole, bm->link);
      if(!bm->link.isEmpty())
        item->setIcon(0, LXDG::findIcon("bookmark-new"));
      else
        item->setIcon(0, LXDG::findIcon("bookmark-remove"));
    }
  }
}
