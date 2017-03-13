//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_PDF_VIEWER_MAIN_WINDOW_H
#define _LUMINA_PDF_VIEWER_MAIN_WINDOW_H

#include <QPrintPreviewDialog>
#include <QPrinter>

#include <poppler-qt5.h>

class MainUI : public QPrintPreviewDialog{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

	void loadFile(QString path);

	virtual void done(int);

private:
	Poppler::Document *DOC;

private slots:
	void paintOnWidget(QPrinter *PRINTER);
	void OpenNewFile();

};
#endif
