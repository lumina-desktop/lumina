//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_PDF_VIEWER_MAIN_WINDOW_H
#define _LUMINA_PDF_VIEWER_MAIN_WINDOW_H

#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QPrintDialog>
#include <QPrintPreviewWidget>
#include <QPrinter>
#include <QProgressBar>
#include <QTimer>
#include <QWheelEvent>

#include "BookmarkMenu.h"
#include "PresentationLabel.h"
#include "PrintWidget.h"
#include "PropDialog.h"
#include "Renderer.h"
#include "TextData.h"

namespace Ui {
class MainUI;
};

class MainUI : public QMainWindow {
  Q_OBJECT
public:
  MainUI();
  ~MainUI();

  void loadFile(QString path);

private:
  QSizeF pageSize;
  PrintWidget *WIDGET;
  Ui::MainUI *ui;
  PropDialog *PROPDIALOG;
  BookmarkMenu *BOOKMARKS;
  QPrintDialog *PrintDLG;
  QString lastdir;
  bool matchCase;
  QList<TextData *> results;
  QList<int> loadingQueue;
  int currentHighlight;

  // Other Interface elements
  QProgressBar *progress;
  QAction *progAct; // action associated with the progressbar
  QTimer *clockTimer;
  QMenu *contextMenu;
  // QFrame *frame_presenter;
  QLabel *label_clock, *label_page;
  QAction *clockAct, *pageAct;

  // PDF Page Loading cache variables
  Renderer *BACKEND;

  // Functions/variables for the presentation mode
  PresentationLabel *presentationLabel;
  QScreen *getScreen(bool current, bool &cancelled);
  int CurrentPage;
  void startPresentation(bool atStart);
  void ShowPage(int page);
  void endPresentation();

private slots:
  void startLoadingPages(int degrees = 0);
  void slotPageLoaded(int);
  void slotSetProgress(int);

  // Simplification routines
  void nextPage() {
    ShowPage(WIDGET->currentPage() + 1);
  } // currentPage() starts at 1 rather than 0
  void prevPage() {
    ShowPage(WIDGET->currentPage() - 1);
  } // currentPage() starts at 1 rather than 0
  void firstPage() { ShowPage(1); }
  void lastPage() { ShowPage(BACKEND->numPages()); }
  void startPresentationHere() { startPresentation(false); }
  void startPresentationBeginning() { startPresentation(true); }
  void closePresentation() { endPresentation(); }

  void find(QString text, bool forward);

  void paintToPrinter(QPrinter *PRINTER);

  // Button Slots
  void OpenNewFile();

  // Other interface slots
  void updateClock();
  void updatePageNumber();
  void showContextMenu(const QPoint &) { contextMenu->popup(QCursor::pos()); }
  void updateContextMenu();
  // void setScroll(bool);

protected:
  void keyPressEvent(QKeyEvent *);
  void wheelEvent(QWheelEvent *);
  void closeEvent(QCloseEvent *ev) {
    endPresentation();
    QMainWindow::closeEvent(ev);
  }
};
#endif
