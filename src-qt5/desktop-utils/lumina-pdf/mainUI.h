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

#include <poppler-qt5.h>

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

	//PDF Page Loading cache variables
	QHash<int, QImage> loadingHash;
	int numPages;

	void loadPage(int num, Poppler::Document *doc, MainUI *obj, QSize dpi, QSizeF page);

	//Functions/variables for the presentation mode
	QLabel *presentationLabel;
	QScreen *getScreen(bool current, bool &cancelled);
	int CurrentPage;
	void startPresentation(bool atStart);
	void ShowPage(int page);
	void endPresentation();

private slots:
	void startLoadingPages(QPrinter *printer);
	void slotPageLoaded(int);
	void slotStartPresentation(QAction *act);

	void paintOnWidget(QPrinter *PRINTER);
	void paintToPrinter(QPrinter *PRINTER);

	//Button Slots
	void OpenNewFile();

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
	      endPresentation();
	    }else if(event->key()==Qt::Key_Right || event->key()==Qt::Key_Down || event->key()==Qt::Key_Space || event->key()==Qt::Key_PageDown){
	      //qDebug() << " - Right/Down/Spacebar" << inPresentation;
	      ShowPage( WIDGET->currentPage() ); //currentPage() starts at 1 rather than 0
	    }else if(event->key()==Qt::Key_Left || event->key()==Qt::Key_Up || event->key()==Qt::Key_PageUp){
	      //qDebug() << " - Left/Up";
	      ShowPage( WIDGET->currentPage()-2 ); //currentPage() starts at 1 rather than 0
	    }else if(event->key()==Qt::Key_Home){
	      //qDebug() << " - Home";
	      ShowPage(0); //go to the first page
	    }else if(event->key()==Qt::Key_End){
	      //qDebug() << " - End";
	      ShowPage( numPages-1 ); //go to the last page
	    }else if(event->key()==Qt::Key_F11){
	      //qDebug() << " - F11";
	      endPresentation();
            }else{
	      QMainWindow::keyPressEvent(event);
	    }
	}
};
#endif
