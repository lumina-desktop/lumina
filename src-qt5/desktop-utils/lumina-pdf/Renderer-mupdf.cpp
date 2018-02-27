#include "Renderer.h"
#include <mupdf/fitz.h>
#include <mupdf/pdf.h>

#include <QDateTime>

fz_document *DOC;
pdf_document *PDOC;
fz_context *CTX;

inline QString getTextInfo(pdf_obj *info, QString str) {
  pdf_obj *obj = pdf_dict_gets(CTX, info, str.toLatin1().data());
  if(obj){ return pdf_to_utf8(CTX, obj); }
  return "";
}

Renderer::Renderer(){
  DOC = 0;
  CTX = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
  PDOC = 0;
  needpass = false;
}

Renderer::~Renderer(){

}

bool Renderer::loadMultiThread(){ return false; }

QJsonObject Renderer::properties(){
  QJsonObject jobj;
  pdf_obj *info = pdf_dict_gets(CTX, pdf_trailer(CTX, PDOC), "Info");
  jobj.insert("title", getTextInfo(info, "Title") );
  jobj.insert("subject", getTextInfo(info, "Subject") );
  jobj.insert("author", getTextInfo(info, "Author") );
  jobj.insert("creator", getTextInfo(info, "Creator") );
  jobj.insert("producer", getTextInfo(info, "Producer") );
  jobj.insert("keywords", getTextInfo(info, "Keywords") );
  jobj.insert("dt_created", QDateTime::fromString( getTextInfo(info, "CreationDate").left(16), "'D:'yyyyMMddHHmmss").toString() );
  jobj.insert("dt_modified", QDateTime::fromString( getTextInfo(info, "ModDate").left(16), "'D:'yyyyMMddHHmmss").toString() );
  return jobj;
}

bool Renderer::loadDocument(QString path, QString password){
  if(DOC==0){ fz_register_document_handlers(CTX); } //first time through
  else if(path!=docpath){
    //fz_close_document(DOC);
    //Clear out the old document first
    delete DOC;
    DOC=0;
    if(PDOC!=0){ delete PDOC; PDOC=0; }
    needpass = false;
  }
  if(DOC==0){
    DOC = fz_open_document(CTX, path.toLocal8Bit().data());
    if(DOC==0){
      qDebug() << "Could not open file:" << path;
      return false;
    }
    needpass = (fz_needs_password(CTX, DOC) != 0);
  }
  if(PDOC==0){
    PDOC = pdf_open_document(CTX, path.toLocal8Bit().data());
  }

  if(needpass && password.isEmpty()){ return false; }
  else if(needpass){
    needpass = !fz_authenticate_password(CTX, DOC, password.toLocal8Bit());
    if(needpass){ return false; } //incorrect password
  }

  //Could read/access the PDF - go ahead and load the info now
  pnum = -1;
  doctitle.clear();
  //qDebug() << "Opening File:" << path;
  pdf_obj *info = pdf_dict_gets(CTX, pdf_trailer(CTX, (pdf_document*)DOC), "Info");
  if(info) {
    if(pdf_obj *obj = pdf_dict_gets(CTX, info, (char *)"Title")){ doctitle = pdf_to_utf8(CTX, obj); }
  }
  if(doctitle.isEmpty()){ doctitle = path.section("/",-1); }
  pnum = fz_count_pages(CTX, DOC);
  //Setup the Document
  fz_page *PAGE = fz_load_page(CTX, DOC, 0);
  if(PAGE!=0){
    /*switch(PAGE->orientation()){
      case Poppler::Page::Landscape:
        WIDGET->setOrientation(QPageLayout::Landscape); break;
      default:
        WIDGET->setOrientation(QPageLayout::Portrait);
    }*/
    delete PAGE;
    return true; //could see the first page
  }
  return false;
}

QImage Renderer::renderPage(int pagenum, QSize DPI){
  //MuPDF uses a scaling factor, not DPI
  fz_page *PAGE = fz_load_page(CTX, DOC, pagenum);
  QImage img;
  if(PAGE!=0){
    fz_matrix matrix;
    //double scaleFactorW = dpi.width()/72.0;
    //double scaleFactorH = dpi.height()/72.0;
    //fz_scale(&matrix, scaleFactorW, scaleFactorH);
    fz_scale(&matrix, 4.0, 4.0); //need to adjust this later according to DPI
    fz_rotate(&matrix, 0.0);
    fz_pixmap *pixmap = fz_new_pixmap_from_page_number(CTX, DOC, pagenum, &matrix, fz_device_rgb(CTX), 0);
    //unsigned char *samples = fz_pixmap_samples(CTX, pixmap);
    img = QImage(pixmap->samples, pixmap->w, pixmap->h, QImage::Format_RGB888); //make the raw image a bit larger than the end result
  }
  if(PAGE!=0){ delete PAGE; }
  qDebug() << "Render Page:" << pagenum << img.size();
  return img;
}

QList<TextData*> Renderer::searchDocument(QString text, bool matchCase){
  fz_rect rectBuffer[1000];
  QList<TextData*> results;
  for(int i = 0; i < pnum; i++) {
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
  }
  return results;
}
