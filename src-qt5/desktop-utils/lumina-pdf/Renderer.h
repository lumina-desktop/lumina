// ================================ // Simple abstraction class between backend renderers
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

class Renderer : public QObject {
Q_OBJECT

private:
  int pnum; //number of pages - set on loading document
  bool needpass;
  QString docpath; //save the path for the currently-loaded document
  QString doctitle;
  QJsonObject jobj;
	int degrees;

public:
  Renderer();
  ~Renderer();
  bool loadMultiThread();

  //Information functions (usually needs to be loaded first)
  int numPages(){ return pnum; }
  bool needPassword(){ return needpass; }
  QString title(){ return doctitle; }
  QJsonObject properties() { return jobj; }
  int hashSize();
  QImage imageHash(int pagenum);
	int rotatedDegrees() { return degrees; }

  //Main access functions
  bool loadDocument(QString path, QString password);
  void renderPage(int pagenum, QSize DPI, int degrees=0);
  QList<TextData*> searchDocument(QString text, bool matchCase);

	void clearHash();
  //Makes sure degrees is between 0 and 360 then rotates the matrix and 
  void setDegrees(int degrees) {
    //Mods by 360, but adds and remods because of how C++ treats negative mods
    this->degrees = ( ( ( this->degrees + degrees ) % 360 ) + 360 ) % 360;
    emit reloadPages(this->degrees);
  }

  bool supportsExtraFeatures();

signals:
  void PageLoaded(int);
  void OrigSize(QSizeF);
	void reloadPages(int);
};

#endif
