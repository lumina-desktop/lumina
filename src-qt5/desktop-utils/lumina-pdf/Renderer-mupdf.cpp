#include "Renderer.h"
#include <QDateTime>
#include <mupdf/fitz.h>
#include <mupdf/pdf.h>
#include <QMutex>
#include <QFuture>
#include <QtConcurrent>

class Data {
  public:
    Data(int _pagenum, fz_context *_ctx, fz_display_list *_list, fz_rect _bbox, fz_pixmap *_pix, fz_matrix _ctm, double _sf) : pagenumber(_pagenum),
      ctx(_ctx),
      list(_list),
      bbox(_bbox),
      pix(_pix),
      ctm(_ctm),
      sf(_sf)
    { }

    ~Data() { }

    int pagenumber;
    fz_context *ctx;
    fz_display_list *list;
    fz_rect bbox;
    fz_pixmap *pix;
    fz_matrix ctm;
    QImage img;
    QFuture<void> renderThread;
    double sf;
};

fz_document *DOC;
fz_context *CTX;
QHash<int, Data*> dataHash;
QMutex mutex[FZ_LOCK_MAX];
fz_locks_context locks;

inline QString getTextInfo(QString str) {
  char infoBuff[1000];
  int size = DOC->lookup_metadata(CTX, DOC, ("info:"+str).toLocal8Bit().data(), infoBuff, 1000);
  if(size != -1){ return QString::fromLatin1(infoBuff); }
  return "";
}

void lock_mutex(void *user, int lock) {
  QMutex *mutex = (QMutex*) user;
  mutex[lock].lock();
}

void unlock_mutex(void *user, int lock) {
  QMutex *mutex = (QMutex*) user;
  mutex[lock].unlock();
}

Renderer::Renderer(){
  locks.user = mutex;
  locks.lock = lock_mutex;
  locks.unlock = unlock_mutex;

  DOC = 0;
  qDebug() << "Creating Context";
  CTX = fz_new_context(NULL, &locks, FZ_STORE_UNLIMITED);
  needpass = false;
  degrees = 0;
}

Renderer::~Renderer(){
  qDebug() << "Dropping Context";
  clearHash();
  fz_drop_document(CTX, DOC);
  DOC = NULL;
  fz_drop_context(CTX);
  CTX = NULL;
}

bool Renderer::loadMultiThread(){ return false; }

void Renderer::handleLink(QString link) {
  float xp = 0.0, yp = 0.0;
  int pagenum = 0;

  QByteArray linkData = link.toLocal8Bit();
  char *uri = linkData.data();

  if(!link.isEmpty()) {
    pagenum = fz_resolve_link(CTX, DOC, uri, &xp, &yp);
    emit goToPosition(pagenum+1, xp, yp);
  }
}

//Traverse the outline tree through Preorder traversal
void Renderer::traverseOutline(void *link, int level) {
  fz_outline *olink = (fz_outline*)link;

  Bookmark *bm = new Bookmark(olink->title, olink->uri, olink->page, level);
  bookmarks.push_back(bm);

  if(olink->down)
    traverseOutline(olink->down, level+1);
  
  if(olink->next)
    traverseOutline(olink->next, level); 
}

bool Renderer::loadDocument(QString path, QString password){
  //first time through
  if(path != docpath) {
    if(DOC != 0) {
      qDebug() << "New document";
      fz_drop_document(CTX, DOC);
      DOC = NULL;
      needpass = false;
      docpath = path;
      if(bookmarks.size() > 0) {
        qDeleteAll(bookmarks);
        bookmarks.clear();
      }
    }else if(DOC==0){ 
      fz_register_document_handlers(CTX); 
      qDebug() << "Document handlers registered";
    }

    docpath = path;
    DOC = fz_open_document(CTX, path.toLocal8Bit().data());
    qDebug() << "File opened" << DOC;
    if(DOC==0){
      qDebug() << "Could not open file:" << path;
      return false;
    }
    needpass = (fz_needs_password(CTX, DOC) != 0);

    if(needpass && password.isEmpty()){ 
      return false; 
    }else if(needpass){
      needpass = !fz_authenticate_password(CTX, DOC, password.toLocal8Bit());
      if(needpass){ return false; } //incorrect password
    }

    qDebug() << "Password Check cleared";
    pnum = fz_count_pages(CTX, DOC);
    qDebug() << "Page count: " << pnum;

    doctitle.clear();

    qDebug() << "Opening File:" << path;
    jobj.insert("subject", getTextInfo("Subject") );
    jobj.insert("author", getTextInfo("Author") );
    jobj.insert("creator", getTextInfo("Creator") );
    jobj.insert("producer", getTextInfo("Producer") );
    jobj.insert("keywords", getTextInfo("Keywords") );
    jobj.insert("dt_created", QDateTime::fromString( getTextInfo("CreationDate").left(16), "'D:'yyyyMMddHHmmss").toString() );
    jobj.insert("dt_modified", QDateTime::fromString( getTextInfo("ModDate").left(16), "'D:'yyyyMMddHHmmss").toString() );

    if(!jobj["title"].toString().isEmpty())
      doctitle = jobj["title"].toString();
    else
      doctitle = path.section("/",-1);

    //Possibly check Page orientation

    fz_outline *outline = fz_load_outline(CTX, DOC);
    if(outline) 
      traverseOutline(outline, 0);
    else
      qDebug() << "No Bookmarks";

    fz_drop_outline(CTX, outline);

    return true;
  }
  return false;
}

void renderer(Data *data, Renderer *obj)
{
  int pagenum = data->pagenumber;
  //qDebug() << "Rendering:" << pagenum;
  fz_context *ctx = data->ctx;
  fz_display_list *list = data->list;
  fz_rect bbox = data->bbox;
  fz_pixmap *pixmap = data->pix;
  fz_matrix ctm = data->ctm;
  fz_device *dev;

  ctx = fz_clone_context(ctx);
  dev = fz_new_draw_device(ctx, &fz_identity, pixmap);
  fz_run_display_list(ctx, list, dev, &ctm, &bbox, NULL);

  data->img = QImage(pixmap->samples, pixmap->w, pixmap->h, pixmap->stride, QImage::Format_RGB888);   
  fz_close_device(ctx, dev);
  fz_drop_device(ctx, dev);

  fz_drop_context(ctx);

  if(dataHash.find(pagenum) == dataHash.end())
    dataHash.insert(pagenum, data);
  else
    dataHash[pagenum] = data;
  emit obj->PageLoaded(pagenum);
}

void Renderer::renderPage(int pagenum, QSize DPI, int degrees){
  //qDebug() << "- Rendering Page:" << pagenum << degrees;
  Data *data;
  fz_matrix matrix;
  fz_rect bbox;
  fz_irect rbox;
  fz_pixmap *pixmap;
  fz_display_list *list;

  double pageDPI = 96.0;
  double sf = DPI.width() / pageDPI;
  fz_scale(&matrix, sf, sf);
  fz_pre_rotate(&matrix, degrees);

  fz_page *PAGE = fz_load_page(CTX, DOC, pagenum);
  fz_bound_page(CTX, PAGE, &bbox);
  emit OrigSize(QSizeF(bbox.x1 - bbox.x0, bbox.y1 - bbox.y0));

  fz_transform_rect(&bbox, &matrix);
  list = fz_new_display_list(CTX, &bbox);
  fz_device *dev = fz_new_list_device(CTX, list);
  fz_run_page(CTX, PAGE, dev, &fz_identity, NULL);  
  /*fz_annot *annot = fz_first_annot(CTX, PAGE);

  while(annot) {
    fz_run_annot(CTX, annot, dev, &fz_identity, NULL);
    fz_rect bbox;
    fz_bound_annot(CTX, annot, &bbox);
    
    QRectF rect(bbox.x0, bbox.y0, (bbox.x1-bbox.x0), (bbox.y1 - bbox.y0));
    qDebug() << "Annotation:" << rect << "at" << pagenum;
    annot = fz_next_annot(CTX, annot);
  }*/

  fz_close_device(CTX, dev);
  fz_drop_device(CTX, dev);
  fz_drop_page(CTX, PAGE);

  pixmap = fz_new_pixmap_with_bbox(CTX, fz_device_rgb(CTX), fz_round_rect(&rbox, &bbox), NULL, 0);
  fz_clear_pixmap_with_value(CTX, pixmap, 0xff);

  data = new Data(pagenum, CTX, list, bbox, pixmap, matrix, sf);
  data->renderThread = QtConcurrent::run(&renderer, data, this);
}

QList<TextData*> Renderer::searchDocument(QString text, bool matchCase){
  fz_rect rectBuffer[1000];
  QList<TextData*> results;
  for(int i = 0; i < pnum; i++) {
    int count = fz_search_display_list(CTX, dataHash[i]->list, text.toLocal8Bit().data(), rectBuffer, 1000);
    //qDebug() << "Page " << i+1 << ": Count, " << count;
    for(int j = 0; j < count; j++) {
      double sf = dataHash[i]->sf;
      QRectF rect(rectBuffer[j].x0*sf, rectBuffer[j].y0*sf, (rectBuffer[j].x1-rectBuffer[j].x0)*sf, (rectBuffer[j].y1 - rectBuffer[j].y0)*sf);
      TextData *t = new TextData(rect, i+1, text);
      //MuPDF search does not match case, so retrieve the exact text at the location found and determine whether or not it matches the case of the search text if the user selected to match case
      if(matchCase){
        fz_stext_page *sPage = fz_new_stext_page_from_display_list(CTX, dataHash[i]->list, NULL);
        QString currentStr = QString(fz_copy_selection(CTX, sPage, *fz_rect_min(&rectBuffer[j]), *fz_rect_max(&rectBuffer[j]), false));
        if(currentStr.contains(text, Qt::CaseSensitive)){ results.append(t); }
      }else{
          results.append(t);
      }
    }
  }
  return results;
}

QImage Renderer::imageHash(int pagenum) {
  return dataHash[pagenum]->img;
}

int Renderer::hashSize() {
  return dataHash.size();
}

void Renderer::clearHash() {
  for(int i = 0; i < dataHash.size(); i++) {
    fz_drop_pixmap(CTX, dataHash[i]->pix);
    fz_drop_display_list(CTX, dataHash[i]->list);
  }
  qDeleteAll(dataHash);
  dataHash.clear();
}

bool Renderer::supportsExtraFeatures() { return true; }
