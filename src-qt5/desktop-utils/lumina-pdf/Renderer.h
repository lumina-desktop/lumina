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
#include <QMessageBox>
#include <QProcess>
#include "TextData.h"
#include "Bookmark.h"
#include "Annotation.h"
#include "Widget.h"

class Renderer : public QObject {
Q_OBJECT

private:
  int pnum; //number of pages - set on loading document
  bool needpass;
  QString docpath; //save the path for the currently-loaded document
  QString doctitle;
  QJsonObject jobj;
	int degrees;
  QList<Bookmark*> bookmarks;

public:
  Renderer();
  ~Renderer();
  bool loadMultiThread();

  //Information functions (usually needs to be loaded first)
  virtual int numPages(){ return pnum; }
  virtual bool needPassword(){ return needpass; }
  virtual QString title(){ return doctitle; }
  virtual QJsonObject properties() { return jobj; }
  virtual int hashSize();
  virtual QImage imageHash(int pagenum);
	virtual int rotatedDegrees() { return degrees; }
  virtual QList<Bookmark*> getBookmarks() { return bookmarks; }

  //Main access functions
  virtual bool loadDocument(QString path, QString password);
  virtual void renderPage(int pagenum, QSize DPI, int degrees=0);
  virtual QList<TextData*> searchDocument(QString text, bool matchCase);
  virtual void traverseOutline(void *, int);
  virtual void handleLink(QWidget*, QString);
  virtual TextData *linkList(int, int);
  virtual int linkSize(int);
  virtual Annotation *annotList(int, int);
  virtual int annotSize(int);
  virtual Widget *widgetList(int, int);
  virtual int widgetSize(int);

	virtual void clearHash();
  //Makes sure degrees is between 0 and 360 then rotates the matrix and 
  void setDegrees(int degrees) {
    //Mods by 360, but adds and remods because of how C++ treats negative mods
    this->degrees = ( ( ( this->degrees + degrees ) % 360 ) + 360 ) % 360;
    emit reloadPages(this->degrees);
  }

  virtual bool supportsExtraFeatures();

signals:
  void PageLoaded(int);
  void OrigSize(QSizeF);
	void reloadPages(int);
  void goToPosition(int, float, float);
};

#endif
