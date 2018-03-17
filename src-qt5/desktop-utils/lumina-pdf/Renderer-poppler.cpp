#include "Renderer.h"
#include <poppler/qt5/poppler-qt5.h>
#include <QThread>

static Poppler::Document *DOC;
QHash<int, QImage> loadingHash;

Renderer::Renderer(){
  DOC = 0;
  needpass = false;
  pnum = 0;
}

Renderer::~Renderer(){
	//qDeleteAll(loadingHash);
	loadingHash.clear();
}

bool Renderer::loadMultiThread(){ return true; }

/*QJsonObject Renderer::properties(){
  return QJsonObject(); //TO-DO
}*/

bool Renderer::loadDocument(QString path, QString password){
  //qDebug() << "Load Document:" << path;
  if(DOC!=0 && path!=docpath){
    //Clear out the old document first
    delete DOC;
    DOC=0;
    needpass = false;
    pnum=0;
		docpath = path;
  }
  //Load the Document (if needed);
  if(DOC==0){
    //qDebug() << "Loading Document";
    DOC = Poppler::Document::load(path);
		docpath = path;
  }

  if(DOC==0){
    qDebug() << "Could not open file:" << path;
    return false;
  }else if(DOC->isLocked()){
    //qDebug() << "Document Locked";
    needpass = true;
    if(password.isEmpty()){ return false; } //stop here - need to get password from user before continuing
    needpass = !DOC->unlock(QByteArray(), password.toLocal8Bit());
    if(needpass){ return false; } //invalid password
  }
  //qDebug() << "Opening File:" << path;
  doctitle = DOC->title();
  if(doctitle.isEmpty()){ doctitle = path.section("/",-1); }
  pnum = DOC->numPages();
  //Setup the Document
  Poppler::Page *PAGE = DOC->page(0);
  if(PAGE!=0){
    /*switch(PAGE->orientation()){
      case Poppler::Page::Landscape:
        WIDGET->setOrientation(QPageLayout::Landscape); break;
      default:
        WIDGET->setOrientation(QPageLayout::Portrait);
    }*/
    delete PAGE;
    return true; //could load the first page
  }
  return false; //nothing to load
}

void Renderer::cleanup() {}

void Renderer::renderPage(int pagenum, QSize DPI){
  //qDebug() << "Render Page:" << pagenum << DPI;
  if(DOC!=0){
		Poppler::Page *PAGE = DOC->page(pagenum);
		QImage img;
		if(PAGE!=0){
			//qDebug() << "Render Page:" << pagenum;
			img = PAGE->renderToImage(DPI.width(),DPI.height());
			loadingHash.insert(pagenum, img);
			//qDebug() << "Image after creation:" << img.isNull();
			delete PAGE;
		}
		//qDebug() << "Done Render Page:" << pagenum << img.size();
	}else{
		loadingHash.insert(pagenum, QImage());
	}
	emit PageLoaded(pagenum);
}

QList<TextData*> Renderer::searchDocument(QString text, bool matchCase){
  QList<TextData*> results;
  for(int i = 0; i < pnum; i++) {
		QList<Poppler::TextBox*> textList = DOC->page(i)->textList();
    for(int j = 0; j < textList.size(); j++) {
			if(textList[j]->text().contains(text, 
			(matchCase) ? Qt::CaseSensitive : Qt::CaseInsensitive)) {
				TextData *t = new TextData(textList[j]->boundingBox(), i+1, text);
				results.append(t);
			}
    }
  }
  return results;
}

QImage Renderer::imageHash(int pagenum) {
	return loadingHash[pagenum];
}

int Renderer::hashSize() {
	return loadingHash.keys().length();
}

void Renderer::clearHash() {
	loadingHash.clear();
}
