// ================================
// Simple abstraction class between backend renderers
// ================================
// Written by Ken Moore: Feb 26, 2018
// Available under the 3-Clause BSD License
// ================================
#ifndef _LUMINA_PDF_BACKEND_RENDERER_H
#define _LUMINA_PDF_BACKEND_RENDERER_H

#include <QString>
#include <QImage>
#include <QDebug>
#include <QJsonObject>
#include "textData.h"

class Renderer{
private:
	int pnum; //number of pages - set on loading document
	bool needpass;
	QString docpath; //save the path for the currently-loaded document
	QString doctitle;
public:
	Renderer();
	~Renderer();
	bool loadMultiThread();

	//Information functions (usually needs to be loaded first)
	int numPages(){ return pnum; }
	bool needPassword(){ return needpass; }
	QString title(){ return doctitle; }
	QJsonObject properties();

	//Main access functions
	bool loadDocument(QString path, QString password);
	QImage renderPage(int pagenum, QSize DPI);
	QList<TextData*> searchDocument(QString text, bool matchCase);
};

#endif
