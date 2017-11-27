//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_PDF_VIEWER_MAIN_WINDOW_H
#define _LUMINA_PDF_VIEWER_MAIN_WINDOW_H

#include <QPrintPreviewWidget>
#include <QPrintDialog>
#include <QPrinter>
#include <QMainWindow>
#include <QProgressBar>
#include <QLabel>
#include <QKeyEvent>
#include <QDebug>
#include <QWheelEvent>
#include <QApplication>
#include <QMenu>

#include <poppler-qt5.h>
#include "PresentationLabel.h"

namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

	void loadFile(QString path);


private:
	Poppler::Document *DOC;
	QPrintPreviewWidget *WIDGET;
	Ui::MainUI *ui;
	QPrinter* Printer;
	QPrintDialog *PrintDLG;

	QString lastdir;

	//Other Interface elements
	QProgressBar *progress;
	QAction *progAct; //action associated with the progressbar
	QTimer *clockTimer;
	QMenu *contextMenu;
	//QFrame *frame_presenter;
	QLabel *label_clock;
	QAction *clockAct;

	//PDF Page Loading cache variables
	QHash<int, QImage> loadingHash;
	int numPages;

	void loadPage(int num, Poppler::Document *doc, MainUI *obj, QSize dpi, QSizeF page);

	//Functions/variables for the presentation mode
	PresentationLabel *presentationLabel;
	QScreen *getScreen(bool current, bool &cancelled);
	int CurrentPage;
	void startPresentation(bool atStart);
	void ShowPage(int page);
	void endPresentation();

private slots:
	void startLoadingPages(QPrinter *printer);
	void slotPageLoaded(int);
	void slotStartPresentation(QAction *act);

	//Simplification routines
	void nextPage(){ ShowPage( WIDGET->currentPage() ); } //currentPage() starts at 1 rather than 0
	void prevPage(){ ShowPage( WIDGET->currentPage()-2 ); } //currentPage() starts at 1 rather than 0
	void firstPage(){ ShowPage(0); }
	void lastPage(){ ShowPage(numPages-1); }
	void startPresentationHere(){ startPresentation(false); }
	void startPresentationBeginning(){ startPresentation(true); }
	void closePresentation(){ endPresentation(); }


	void paintOnWidget(QPrinter *PRINTER);
	void paintToPrinter(QPrinter *PRINTER);

	//Button Slots
	void OpenNewFile();

	//Other interface slots
	void updateClock();
	void showContextMenu(const QPoint&){ contextMenu->popup(QCursor::pos()); }
	void updateContextMenu();

signals:
	void PageLoaded(int);

protected:
	void keyPressEvent(QKeyEvent *event){
	  //See if this is one of the special hotkeys and act appropriately
	    //qDebug() << "Got Key Press:";
	  bool inPresentation = (presentationLabel!=0);
 	  if(!inPresentation){
	    //Alternate functionality when **not** in presentation mode
	    /*if(event->key()==Qt::Key_Down){
	      qDebug() << "Send Wheel Event";
 	      QWheelEvent event( WIDGET->mapFromGlobal(QCursor::pos()), QCursor::pos(),QPoint(0,0), QPoint(0,30), 0, Qt::Vertical, Qt::LeftButton, Qt::NoModifier);
	      QApplication::sendEvent(WIDGET, &event);
	      //WIDGET->scrollDown();
 	      return;
	    }else if(event->key()==Qt::Key_Up){
	      return;
	    }*/
	  }

	    if( event->key()==Qt::Key_Escape || event->key()==Qt::Key_Backspace){
	      //qDebug() << " - Escape/Backspace";
	      if(inPresentation){ endPresentation(); }
	    }else if(event->key()==Qt::Key_Right || event->key()==Qt::Key_Down || event->key()==Qt::Key_Space || event->key()==Qt::Key_PageDown){
	      //qDebug() << " - Right/Down/Spacebar" << inPresentation;
	      nextPage();
	    }else if(event->key()==Qt::Key_Left || event->key()==Qt::Key_Up || event->key()==Qt::Key_PageUp){
	      //qDebug() << " - Left/Up";
	      prevPage();
	    }else if(event->key()==Qt::Key_Home){
	      //qDebug() << " - Home";
	      firstPage();
	    }else if(event->key()==Qt::Key_End){
	      //qDebug() << " - End";
	      lastPage();
	    }else if(event->key()==Qt::Key_F11){
	      //qDebug() << " - F11";
	      if(inPresentation){ endPresentation(); }
	      else{ startPresentationHere(); }
            }else{
	      QMainWindow::keyPressEvent(event);
	    }
	}
};
#endif
