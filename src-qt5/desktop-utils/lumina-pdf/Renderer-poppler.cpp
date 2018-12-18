#include "Renderer.h"
#include "drawablepage.h"
#include "link.h"
#include "lrucache.h"
#include <QThread>
#include <atomic>
#include <poppler/qt5/poppler-qt5.h>

static std::unique_ptr<Poppler::Document> DOC;
QHash<int, QImage> loadingHash;

static std::vector<LuminaPDF::drawablePage> pages;
static std::vector<QList<LuminaPDF::Link *>> links;
static std::atomic<int> pagesStillLoading;
static QHash<int, QList<LuminaPDF::Link *>> linkHash;
static LuminaPDF::LRUCache<QImage> imageCache;

Renderer::Renderer() : pnum(0), needpass(false), degrees(0) {
  DOC.reset(nullptr);
  pagesStillLoading = 1;
  imageCache.setCacheSize(5);
}

Renderer::~Renderer() {
  // qDeleteAll(loadingHash);
  pages.clear();
  for (auto &linkList : links) {
    qDeleteAll(linkList);
  }
  loadingHash.clear();
}

bool Renderer::loadMultiThread() { return true; }

/*QJsonObject Renderer::properties(){
  return QJsonObject(); //TO-DO
}*/

bool Renderer::loadDocument(QString path, QString password) {
  // qDebug() << "Load Document:" << path;
  if (DOC != nullptr && path != docpath) {
    // Clear out the old document first
    DOC.reset(nullptr);
    pages.clear();
    if (linkHash.size() > 0) {
      foreach (QList<LuminaPDF::Link *> linkArray, linkHash) {
        qDeleteAll(linkArray);
      }
      linkHash.clear();
    }
    needpass = false;
    pnum = 0;
    docpath = path;
  }
  // Load the Document (if needed);
  if (DOC == nullptr) {
    // qDebug() << "Loading Document";
    DOC.reset(Poppler::Document::load(path));
    docpath = path;
  }

  if (DOC == nullptr) {
    qDebug() << "Could not open file:" << path;
    return false;
  }

  if (DOC->isLocked()) {
    // qDebug() << "Document Locked";
    needpass = true;
    if (password.isEmpty() or
        !DOC->unlock(QByteArray(), password.toLocal8Bit())) {
      return false;
    } // invalid password
  }
  // qDebug() << "Opening File:" << path;
  doctitle = DOC->title();
  if (doctitle.isEmpty()) {
    doctitle = path.section("/", -1);
  }
  pnum = DOC->numPages();
  // Setup the Document
  Poppler::Page *PAGE = DOC->page(0);
  if (PAGE != 0) {
    /*switch(PAGE->orientation()){
      case Poppler::Page::Landscape:
        WIDGET->setOrientation(QPageLayout::Landscape); break;
      default:
        WIDGET->setOrientation(QPageLayout::Portrait);
    }*/
    delete PAGE;
    pages.reserve(pnum + 1);

    for (int i = 0; i < pnum + 1; ++i) {
      LuminaPDF::drawablePage temp;
      pages.emplace_back(std::move(temp));
    }

    for (int i = 0; i < pnum + 1; ++i) {
      QList<LuminaPDF::Link *> temp;
      links.push_back(temp);
    }

    pagesStillLoading = pnum;

    return true; // could load the first page
  }
  return false; // nothing to load
}

void Renderer::renderPage(int pagenum, QSize DPI, int degrees) {
  // qDebug() << "Render Page:" << pagenum << DPI << degrees;

  emit SetProgress(pages.size() - pagesStillLoading);

  if (DOC != nullptr) {
    Poppler::Page *PAGE = DOC->page(pagenum - 1);
    QImage img;
    if (PAGE != nullptr) {
      Poppler::Page::Rotation rotation;
      switch (degrees) {
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

      LuminaPDF::drawablePage temp(PAGE, DPI, rotation);
      pages[pagenum] = std::move(temp);
      // img = PAGE->renderToImage(DPI.width(), DPI.height(), -1, -1, -1, -1,
      //                          rotation);
      // loadingHash.insert(pagenum, img);
      QList<LuminaPDF::Link *> linkArray;

      foreach (Poppler::Link *link, PAGE->links()) {
        QString location;
        if (link->linkType() == Poppler::Link::LinkType::Goto)
          location = dynamic_cast<Poppler::LinkGoto *>(link)->fileName();
        else if (link->linkType() == Poppler::Link::LinkType::Goto)
          location = dynamic_cast<Poppler::LinkBrowse *>(link)->url();
        LuminaPDF::Link *newLink = new LuminaPDF::Link(
            new TextData(link->linkArea(), pagenum, location), link);
        linkArray.append(newLink);
      }

      links[pagenum] = linkArray;
      // linkHash.insert(pagenum, linkArray);
    }
    // qDebug() << "Done Render Page:" << pagenum << img.size();
  } else {
    pages[pagenum] = LuminaPDF::drawablePage();
    // loadingHash.insert(pagenum, QImage());
  }

  if (pagesStillLoading == 1) {
    emit PageLoaded();
  }

  --pagesStillLoading;
}

bool Renderer::isDoneLoading() { return pagesStillLoading == 0; }

QList<TextData *> Renderer::searchDocument(QString text, bool matchCase) {
  QList<TextData *> results;
  for (int i = 0; i < pnum; i++) {
    QList<Poppler::TextBox *> textList = DOC->page(i)->textList();
    for (int j = 0; j < textList.size(); j++) {
      if (textList[j]->text().contains(
              text, (matchCase) ? Qt::CaseSensitive : Qt::CaseInsensitive)) {
        TextData *t = new TextData(textList[j]->boundingBox(), i + 1, text);
        results.append(t);
      }
    }
  }
  return results;
}

QSize Renderer::imageSize(int pagenum) { return pages[pagenum].size(); }

QImage Renderer::imageHash(int pagenum) {
  // while(pagesStillLoading > 0) { qDebug() << "pagesStillLoading!\n";}

  std::optional<QImage> cachedImage = imageCache.get(pagenum);

  if (cachedImage.has_value())
    return *cachedImage;

  imageCache.push(pagenum, pages[pagenum].render());
  return *imageCache.get(pagenum);
}

int Renderer::hashSize() {
  qDebug() << "pages contains " << pages.size() << " elements.\n";
  return pages.size();
}

void Renderer::clearHash() {
  loadingHash.clear();
  pages.clear();
}

// Highlighting found text, bookmarks, and page properties disabled for Poppler
bool Renderer::supportsExtraFeatures() { return false; }

void Renderer::traverseOutline(void *, int) {}

void Renderer::handleLink(QWidget *obj, QString linkDest) {
  Poppler::Link *trueLink;
  foreach (QList<LuminaPDF::Link *> linkArray, linkHash) {
    for (int i = 0; i < linkArray.size(); i++) {
      Poppler::Link *link = linkArray[i]->getLink();
      if (link->linkType() == Poppler::Link::LinkType::Browse) {
        if (linkDest == dynamic_cast<Poppler::LinkBrowse *>(link)->url())
          trueLink = link;
      } else if (link->linkType() == Poppler::Link::LinkType::Goto) {
        if (linkDest == dynamic_cast<Poppler::LinkGoto *>(link)->fileName())
          trueLink = link;
      }
    }
  }
  if (trueLink) {
    if (trueLink->linkType() == Poppler::Link::LinkType::Goto)
      emit goToPosition(dynamic_cast<Poppler::LinkGoto *>(trueLink)
                            ->destination()
                            .pageNumber(),
                        0, 0);
    else if (trueLink->linkType() == Poppler::Link::LinkType::Browse) {
      if (QMessageBox::Yes ==
          QMessageBox::question(
              obj, tr("Open External Link?"),
              QString(tr("Do you want to open %1 in the default browser"))
                  .arg(linkDest),
              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
        QProcess::startDetached("firefox \"" + linkDest + "\"");
      }
    }
  }
}

TextData *Renderer::linkList(int pageNum, int entry) {
  if (linkHash[pageNum].size() > 0)
    return linkHash[pageNum][entry]->getData();
  else
    return 0;
}

int Renderer::linkSize(int pageNum) {
  Q_UNUSED(pageNum) return linkHash[pageNum].size();
}

int Renderer::annotSize(int pageNum) { Q_UNUSED(pageNum) return 0; }

Annotation *Renderer::annotList(int pageNum, int entry) {
  Q_UNUSED(pageNum) Q_UNUSED(entry) return NULL;
}

int Renderer::widgetSize(int pageNum) { Q_UNUSED(pageNum) return 0; }

Widget *Renderer::widgetList(int pageNum, int entry) {
  Q_UNUSED(pageNum) Q_UNUSED(entry) return NULL;
}
