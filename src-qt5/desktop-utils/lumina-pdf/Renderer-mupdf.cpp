#include "Renderer.h"
#include <QDateTime>
#include <mupdf/fitz.h>
//#include <pthread.h>

fz_document *DOC;
fz_context *CTX;

inline QString getTextInfo(QString str) {
	char infoBuff[1000];
	int size = DOC->lookup_metadata(CTX, DOC, ("info:"+str).toLocal8Bit().data(), infoBuff, 1000);
  if(size != -1){ return QString::fromLatin1(infoBuff); }
  return "";
}

/*void lock_mutex(void *user, int lock) {
	pthread_mutex_t *mutex = (pthread_mutex_t *) user;
	pthread_mutex_lock(&mutex[lock]);
}

void unlock_mutex(void *user, int lock) {
	pthread_mutex_t *mutex = (pthread_mutex_t *) user;
	pthread_mutex_unlock(&mutex[lock]);
}*/

Renderer::Renderer(){
	/*pthread_mutex_t mutex[FZ_LOCK_MAX];
	fz_locks_context locks;
	locks.user = mutex;
	locks.lock = lock_mutex;
	locks.unlock = unlock_mutex;

	for (int i = 0; i < FZ_LOCK_MAX; i++)
		pthread_mutex_init(&mutex[i], NULL);*/

	mutex = new QMutex();
  DOC = 0;
  //CTX = fz_new_context(NULL, &locks, FZ_STORE_UNLIMITED);
  CTX = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	qDebug() << "Context created";
  needpass = false;
}

Renderer::~Renderer(){

}

//bool Renderer::loadMultiThread(){ return true; }

QJsonObject Renderer::properties(){
  QJsonObject jobj;
  jobj.insert("title", getTextInfo("Title") );
  jobj.insert("subject", getTextInfo("Subject") );
  jobj.insert("author", getTextInfo("Author") );
  jobj.insert("creator", getTextInfo("Creator") );
  jobj.insert("producer", getTextInfo("Producer") );
  jobj.insert("keywords", getTextInfo("Keywords") );
  jobj.insert("dt_created", QDateTime::fromString( getTextInfo("CreationDate").left(16), "'D:'yyyyMMddHHmmss").toString() );
  jobj.insert("dt_modified", QDateTime::fromString( getTextInfo("ModDate").left(16), "'D:'yyyyMMddHHmmss").toString() );
  return jobj;
}

bool Renderer::loadDocument(QString path, QString password){
	//first time through
  if(DOC==0){ 
		fz_register_document_handlers(CTX); 
		qDebug() << "Document handlers registered";
	}else if(path!=docpath){
    //fz_close_document(DOC);
    //Clear out the old document first
    delete DOC;
    DOC=0;
    needpass = false;
		docpath = path;
  }

  if(DOC==0){
    DOC = fz_open_document(CTX, path.toLocal8Bit().data());
		docpath = path;
		qDebug() << "File opened";
    if(DOC==0){
      qDebug() << "Could not open file:" << path;
      return false;
    }
    needpass = (fz_needs_password(CTX, DOC) != 0);
  }

  if(needpass && password.isEmpty()){ return false; }
  else if(needpass){
    needpass = !fz_authenticate_password(CTX, DOC, password.toLocal8Bit());
    if(needpass){ return false; } //incorrect password
  }

	qDebug() << "Password Check cleared";

  //Could read/access the PDF - go ahead and load the info now
  pnum = -1;
  doctitle.clear();
  //qDebug() << "Opening File:" << path;
	QString title = getTextInfo("Subject");
	if(!title.isNull())
		doctitle = title;
	else
		doctitle = path.section("/",-1);
  pnum = fz_count_pages(CTX, DOC);
  //Setup the Document
  fz_page *PAGE = fz_load_page(CTX, DOC, 0);
	qDebug() << "Page Loaded";
	//Possibly check Page orientation";

  return (PAGE);
}

//Consider rendering through a display list
QImage Renderer::renderPage(int pagenum, QSize DPI){
	QImage img;
	fz_matrix matrix;
	fz_pixmap *pixmap = nullptr;

	fz_scale(&matrix, DPI.width()/96.0, DPI.height()/96.0);

	//fz_context *ctx = fz_clone_context(CTX);
	mutex->lock();

	fz_try(CTX) {
		pixmap = fz_new_pixmap_from_page_number(CTX, DOC, pagenum, &matrix, fz_device_rgb(CTX), 0);
	}fz_catch(CTX){
		qDebug() << "Error when rendering page using MuPDF";
	}

	mutex->unlock();

	img = QImage(pixmap->samples, pixmap->w, pixmap->h, pixmap->stride, QImage::Format_RGB888);
  qDebug() << "Render Page:" << pagenum;
  return img;
}

QList<TextData*> Renderer::searchDocument(QString text, bool matchCase){
  fz_rect rectBuffer[1000];
  QList<TextData*> results;
  for(int i = 0; i < pnum; i++) {
    int count = fz_search_page_number(CTX, DOC, i, text.toLatin1().data(), rectBuffer, 1000);
    //qDebug() << "Page " << i+1 << ": Count, " << count;
    for(int j = 0; j < count; j++) {
      QRect rect(rectBuffer[j].x0, rectBuffer[j].y0, rectBuffer[j].x1-rectBuffer[j].x0, rectBuffer[j].y1 - rectBuffer[j].y0);
      TextData *t = new TextData(rect, i+1, text);
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
