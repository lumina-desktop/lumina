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

	//Button Slots
	void OpenNewFile();

signals:
	void PageLoaded(int);

protected:
	void keyPressEvent(QKeyEvent *event){
	  //See if this is one of the special hotkeys and act appropriately
	  // NOTE: Some of this is duplicated with the QShortcut definitions (for non-presentation mode)
	  //  This routine does not always work for the main window viewer due to differing widget focus policies
	  if(presentationLabel!=0 && presentationLabel->isVisible()){
	    //qDebug() << "Got Key Press:";
	    if( event->key()==Qt::Key_Escape || event->key()==Qt::Key_Backspace){
	      //qDebug() << " - Escape/Backspace";
	      endPresentation();
	    }else if(event->key()==Qt::Key_Right || event->key()==Qt::Key_Down || event->key()==Qt::Key_Space){
	      //qDebug() << " - Right/Down/Spacebar";
	      ShowPage( CurrentPage+1 );
	    }else if(event->key()==Qt::Key_Left || event->key()==Qt::Key_Up){
	      //qDebug() << " - Left/Up";
	      ShowPage( CurrentPage-1 );
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
	  }else{
	    QMainWindow::keyPressEvent(event);
	  }
	}
};
#endif
