#include "Renderer.h"
#include <poppler/qt5/poppler-qt5.h>
#include <QThread>

class Link {
  public:
    Link(TextData *_data, Poppler::Link *_link) : data(_data), link(_link) { }
    ~Link() { delete data; }
  
    TextData* getData() { return data; }
    Poppler::Link* getLink() { return link; }

  private:
    TextData *data;
    Poppler::Link *link;
};

static Poppler::Document *DOC;
QHash<int, QImage> loadingHash;
QHash<int, QList<Link*>> linkHash;

Renderer::Renderer(){
  DOC = 0;
  needpass = false;
  pnum = 0;
	degrees = 0;
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
    if(linkHash.size() > 0) {
      foreach(QList<Link*> linkArray, linkHash) {
        qDeleteAll(linkArray);
      }
      linkHash.clear();
    }
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

void Renderer::renderPage(int pagenum, QSize DPI, int degrees){
  //qDebug() << "Render Page:" << pagenum << DPI << degrees;
  if(DOC!=0){
    Poppler::Page *PAGE = DOC->page(pagenum);
    QImage img;
    if(PAGE!=0){
			Poppler::Page::Rotation rotation;
			switch(degrees) {
				case 90:
					rotation = Poppler::Page::Rotation::Rotate90;
					break;
				case 180:
					rotation = Poppler::Page::Rotation::Rotate180;
					break;
				case 270:
					rotation = Poppler::Page::Rotation::Rotate270;
					break;
				default:
					rotation = Poppler::Page::Rotation::Rotate0;
			}
      img = PAGE->renderToImage(DPI.width(), DPI.height(), -1, -1, -1, -1, rotation);
      loadingHash.insert(pagenum, img);
      QList<Link*> linkArray;
      foreach(Poppler::Link *link, PAGE->links()) {
        QString location;
        if(link->linkType() == Poppler::Link::LinkType::Goto)
          location = dynamic_cast<Poppler::LinkGoto*>(link)->fileName();
        else if(link->linkType() == Poppler::Link::LinkType::Goto)
          location = dynamic_cast<Poppler::LinkBrowse*>(link)->url();
        Link *newLink = new Link(new TextData(link->linkArea(), pagenum, location), link);
        linkArray.append(newLink);
      }
      linkHash.insert(pagenum, linkArray);
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

//Highlighting found text, bookmarks, and page properties disabled for Poppler
bool Renderer::supportsExtraFeatures() { return false; }

void Renderer::traverseOutline(void *, int) { }

void Renderer::handleLink(QWidget *obj, QString linkDest) { 
  Poppler::Link* trueLink;
  foreach(QList<Link*> linkArray, linkHash) {
    for(int i = 0; i < linkArray.size(); i++) {
      Poppler::Link* link = linkArray[i]->getLink();
      if(link->linkType() == Poppler::Link::LinkType::Browse) {
        if(linkDest == dynamic_cast<Poppler::LinkBrowse*>(link)->url())
          trueLink = link;
      }else if(link->linkType() == Poppler::Link::LinkType::Goto) {
        if(linkDest == dynamic_cast<Poppler::LinkGoto*>(link)->fileName())
          trueLink = link;
      }
    }
  }
  if(trueLink) {
    if(trueLink->linkType() == Poppler::Link::LinkType::Goto)
      emit goToPosition(dynamic_cast<Poppler::LinkGoto*>(trueLink)->destination().pageNumber(), 0, 0);
    else if(trueLink->linkType() == Poppler::Link::LinkType::Browse) {
      if(QMessageBox::Yes == QMessageBox::question(obj, tr("Open External Link?"), QString(tr("Do you want to open %1 in the default browser")).arg(linkDest), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ){ QProcess::startDetached("firefox \""+linkDest+"\""); }
    }
  }
}

TextData* Renderer::linkList(int pageNum, int entry) { 
  if(linkHash[pageNum].size() > 0)
    return linkHash[pageNum][entry]->getData();
  else
    return 0;
}

int Renderer::linkSize(int pageNum) { Q_UNUSED(pageNum) return linkHash[pageNum].size(); }

int Renderer::annotSize(int pageNum) { Q_UNUSED(pageNum) return 0; }

Annotation *Renderer::annotList(int pageNum, int entry) { Q_UNUSED(pageNum) Q_UNUSED(entry) return NULL; }

int Renderer::widgetSize(int pageNum) { Q_UNUSED(pageNum) return 0; }

Widget *Renderer::widgetList(int pageNum, int entry) { Q_UNUSED(pageNum) Q_UNUSED(entry) return NULL; }
