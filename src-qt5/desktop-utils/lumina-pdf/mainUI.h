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

	
private slots:
	void paintOnWidget(QPrinter *PRINTER);

	//Button Slots
	void OpenNewFile();

};
#endif
