#include "Renderer.h"
#include <poppler/qt5/poppler-qt5.h>

static Poppler::Document *DOC;

Renderer::Renderer(){
  DOC = 0;
  needpass = false;
  pnum = 0;
}

Renderer::~Renderer(){

}

bool Renderer::loadMultiThread(){ return true; }

QJsonObject Renderer::properties(){
  return QJsonObject(); //TO-DO
}

bool Renderer::loadDocument(QString path, QString password){
  //qDebug() << "Load Document:" << path;
  if(DOC!=0 && path!=docpath){
    //Clear out the old document first
    delete DOC;
    DOC=0;
    needpass = false;
    pnum=0;
  }
  //Load the Document (if needed);
  if(DOC==0){
    //qDebug() << "Loading Document";
    DOC = Poppler::Document::load(path);
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

QImage Renderer::renderPage(int pagenum, QSize DPI){
  //qDebug() << "Render Page:" << pagenum << DPI;
  if(DOC==0){ return QImage(); }
  Poppler::Page *PAGE = DOC->page(pagenum);
  QImage img;
  if(PAGE!=0){
    //qDebug() << "Render Page:" << pagenum;
    img = PAGE->renderToImage(DPI.width(),DPI.height());
    delete PAGE;
  }
  //qDebug() << "Done Render Page:" << pagenum << img.size();
  return img;
}

QList<TextData*> Renderer::searchDocument(QString text, bool matchCase){
  QList<TextData*> results;
  /*for(int i = 0; i < pnum; i++) {
    int count = fz_search_page_number(CTX, DOC, i, text.toLatin1().data(), rectBuffer, 1000);
    //qDebug() << "Page " << i+1 << ": Count, " << count;
    for(int j = 0; j < count; j++) {
      TextData *t = new TextData(rectBuffer[j], i+1, text);
      //MuPDF search does not match case, so retrieve the exact text at the location found and determine whether or not it matches the case of the search text if the user selected to match case
      if(matchCase){
        fz_stext_page *sPage = fz_new_stext_page_from_page_number(CTX, DOC, i, NULL);
        QString currentStr = QString(fz_copy_selection(CTX, sPage, *fz_rect_min(&rectBuffer[j]), *fz_rect_max(&rectBuffer[j]), false));
        if(currentStr.contains(text, Qt::CaseSensitive)){ results.append(t); }
      }else{
          results.append(t);
      }
    }
  }*/
  return results;
}
