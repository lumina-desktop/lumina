#include "Renderer.h"
#include "TextData.h"
#include <QDateTime>
#include <QFuture>
#include <QMutex>
#include <QtConcurrent>
#include <mupdf/fitz.h>
#include <mupdf/pdf.h>

QMutex hashMutex;
class Annot;

inline QRectF convertRect(fz_rect bbox, double sf = 1.0) {
  return QRectF(sf * bbox.x0, sf * bbox.y0, sf * (bbox.x1 - bbox.x0),
                sf * (bbox.y1 - bbox.y0));
}

class Link {
public:
  Link(fz_context *_ctx, fz_link *_fzLink, char *_uri, int _page,
       QRectF _loc = QRectF())
      : fzLink(_fzLink), ctx(_ctx) {
    QString uri = QString::fromLocal8Bit(_uri);
    data = new TextData(_loc, _page, uri);
  }

  ~Link() {
    fz_drop_link(ctx, fzLink);
    delete data;
  }

  TextData *getData() { return data; }

private:
  TextData *data;
  fz_link *fzLink;
  fz_context *ctx;
};

class Data {
public:
  Data(int _pagenum, fz_context *_ctx, fz_display_list *_list, fz_rect _bbox,
       fz_matrix _ctm, double _sf, fz_link *_link, QList<Annot *> &_annot,
       QList<Widget *> &_widgets)
      : pagenumber(_pagenum), ctx(_ctx), list(_list), bbox(_bbox), ctm(_ctm),
        sf(_sf), annotList(_annot), widgetList(_widgets) {

    while (_link) {
      QRectF rect = convertRect(_link->rect, sf);
      Link *link = new Link(_ctx, _link, _link->uri, _pagenum, rect);
      linkList.append(link);
      _link = _link->next;
    }
  }

  ~Data() {
    fz_drop_pixmap(ctx, pix);
    fz_drop_display_list(ctx, list);
    qDeleteAll(linkList);
    linkList.clear();
    qDeleteAll(annotList);
    annotList.clear();
    qDeleteAll(widgetList);
    widgetList.clear();
  }

  int getPage() { return pagenumber; }
  QList<Link *> getLinkList() { return linkList; }
  Annot *getAnnotList(int i) { return annotList[i]; }
  Widget *getWidgetList(int i) { return widgetList[i]; }
  int getAnnotSize() { return annotList.size(); }
  int getWidgetSize() { return widgetList.size(); }
  fz_context *getContext() { return ctx; }
  fz_display_list *getDisplayList() { return list; }
  QRectF getScaledRect() { return convertRect(bbox, sf); }
  QRectF getScaledRect(fz_rect &rect) { return convertRect(rect, sf); }
  fz_rect getBoundingBox() { return bbox; }
  fz_matrix getMatrix() { return ctm; }
  QImage getImage() { return img; }

  void setImage(QImage _img) { img = _img; }
  void setRenderThread(QFuture<void> thread) { renderThread = thread; }
  void setPixmap(fz_pixmap *_pix) { pix = _pix; }

private:
  int pagenumber;
  fz_context *ctx;
  fz_display_list *list;
  fz_rect bbox;
  fz_matrix ctm;
  QList<Link *> linkList;
  double sf;
  QList<Annot *> annotList;
  QList<Widget *> widgetList;

  fz_pixmap *pix;
  QImage img;
  QFuture<void> renderThread;
};

fz_document *DOC;
fz_context *CTX;
QHash<int, Data *> dataHash;
QMutex mutex[FZ_LOCK_MAX];
fz_locks_context locks;

class Annot : public Annotation {
public:
  Annot(fz_context *_ctx, pdf_annot *_fzAnnot, int _type, int _i,
        float _opacity, QRectF _loc = QRectF())
      : Annotation(_type, _opacity, _loc), ctx(_ctx), fzAnnot(_fzAnnot),
        pageNum(_i) {}
  virtual ~Annot() { fz_drop_annot(ctx, (fz_annot *)fzAnnot); }

  virtual QImage renderImage() {
    fz_rect bbox;
    fz_irect rbox;
    pdf_bound_annot(ctx, fzAnnot, &bbox);

    fz_pixmap *pixmap = fz_new_pixmap_with_bbox(
        ctx, fz_device_rgb(ctx), fz_round_rect(&rbox, &bbox), NULL, 0);
    fz_clear_pixmap_with_value(ctx, pixmap, 0xff);
    fz_device *dev = fz_new_draw_device(ctx, &fz_identity, pixmap);
    fz_run_display_list(ctx, dataHash[pageNum]->getDisplayList(), dev,
                        &fz_identity, &bbox, NULL);

    QImage image(pixmap->samples, pixmap->w, pixmap->h, pixmap->stride,
                 QImage::Format_RGB888);
    return image;
  }

private:
  fz_context *ctx;
  pdf_annot *fzAnnot;

  int pageNum;
};

inline QString getTextInfo(QString str) {
  char infoBuff[1000];
  int size = DOC->lookup_metadata(
      CTX, DOC, ("info:" + str).toLocal8Bit().data(), infoBuff, 1000);
  if (size != -1) {
    return QString::fromLatin1(infoBuff);
  }
  return "";
}

void lock_mutex(void *user, int lock) {
  QMutex *mutex = (QMutex *)user;
  mutex[lock].lock();
}

void unlock_mutex(void *user, int lock) {
  QMutex *mutex = (QMutex *)user;
  mutex[lock].unlock();
}

Renderer::Renderer() {
  locks.user = mutex;
  locks.lock = lock_mutex;
  locks.unlock = unlock_mutex;

  DOC = 0;
  // qDebug() << "Creating Context";
  CTX = fz_new_context(NULL, &locks, FZ_STORE_UNLIMITED);
  needpass = false;
  degrees = 0;
}

Renderer::~Renderer() {
  // pdf_clean_page_contents
  // qDebug() << "Dropping Context";
  clearHash();
  fz_drop_document(CTX, DOC);
  DOC = NULL;
  fz_drop_context(CTX);
  CTX = NULL;
}

bool Renderer::loadMultiThread() { return false; }

void Renderer::handleLink(QWidget *obj, QString link) {
  //qDebug() << "handle link";
  float xp = 0.0, yp = 0.0;
  int pagenum = 0;

  QByteArray linkData = link.toLocal8Bit();
  char *uri = linkData.data();

  if (!link.isEmpty()) {
    if (fz_is_external_link(CTX, uri)) {
      if (QMessageBox::Yes ==
          QMessageBox::question(
              obj, tr("Open External Link?"),
              QString(tr("Do you want to open %1 in the default browser"))
                  .arg(link),
              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
        QProcess::startDetached("firefox \"" + link + "\"");
      }
    } else {
      pagenum = fz_resolve_link(CTX, DOC, uri, &xp, &yp);
      if (pagenum != -1)
        emit goToPosition(pagenum + 1, xp, yp);
    }
  }
}

// Traverse the outline tree through Preorder traversal
void Renderer::traverseOutline(void *link, int level) {
  //qDebug() << "traverse outline";
  fz_outline *olink = (fz_outline *)link;

  Bookmark *bm = new Bookmark(olink->title, olink->uri, olink->page, level);
  bookmarks.push_back(bm);

  if (olink->down)
    traverseOutline(olink->down, level + 1);

  if (olink->next)
    traverseOutline(olink->next, level);
}

bool Renderer::loadDocument(QString path, QString password) {
  // first time through
  //qDebug() << "Load Document:" << path;
  if (path != docpath) {
    if (DOC != 0) {
      //qDebug() << "New document";
      fz_drop_document(CTX, DOC);
      DOC = NULL;
      needpass = false;
      docpath = path;
      if (bookmarks.size() > 0) {
        qDeleteAll(bookmarks);
        bookmarks.clear();
      }
    } else if (DOC == 0) {
      fz_register_document_handlers(CTX);
      //qDebug() << "Document handlers registered";
    }

    // fz_page_presentation

    docpath = path;
    DOC = fz_open_document(CTX, path.toLocal8Bit().data());
    // qDebug() << "File opened" << DOC;
    if (DOC == 0) {
      qDebug() << "Could not open file:" << path;
      return false;
    }
    needpass = (fz_needs_password(CTX, DOC) != 0);

    if (needpass && password.isEmpty()) {
      return false;
    } else if (needpass) {
      needpass = !fz_authenticate_password(CTX, DOC, password.toLocal8Bit());
      if (needpass) {
        return false;
      } // incorrect password
    }

    // qDebug() << "Password Check cleared";
    pnum = fz_count_pages(CTX, DOC);
    //qDebug() << "Page count: " << pnum;

    doctitle.clear();

    // qDebug() << "Opening File:" << path;
    jobj.insert("subject", getTextInfo("Subject"));
    jobj.insert("author", getTextInfo("Author"));
    jobj.insert("creator", getTextInfo("Creator"));
    jobj.insert("producer", getTextInfo("Producer"));
    jobj.insert("keywords", getTextInfo("Keywords"));
    jobj.insert("dt_created",
                QDateTime::fromString(getTextInfo("CreationDate").left(16),
                                      "'D:'yyyyMMddHHmmss")
                    .toString());
    jobj.insert("dt_modified",
                QDateTime::fromString(getTextInfo("ModDate").left(16),
                                      "'D:'yyyyMMddHHmmss")
                    .toString());

    if (!jobj["title"].toString().isEmpty())
      doctitle = jobj["title"].toString();
    else
      doctitle = path.section("/", -1);

    // Possibly check Page orientation

    fz_outline *outline = fz_load_outline(CTX, DOC);
    if (outline)
      traverseOutline(outline, 0);
    // else
    // qDebug() << "No Bookmarks";

    fz_drop_outline(CTX, outline);

    return true;
  }
  return false;
}

void renderer(Data *data, Renderer *obj) {
  int pagenum = data->getPage();
  //qDebug() << "Rendering:" << pagenum;
  fz_context *ctx = data->getContext();
  fz_rect bbox = data->getBoundingBox();
  fz_matrix ctm = data->getMatrix();
  fz_device *dev;
  fz_irect rbox;

  ctx = fz_clone_context(ctx);
  fz_pixmap *pixmap = fz_new_pixmap_with_bbox(
      ctx, fz_device_rgb(ctx), fz_round_rect(&rbox, &bbox), NULL, 0);
  fz_clear_pixmap_with_value(ctx, pixmap, 0xff);

  dev = fz_new_draw_device(ctx, &fz_identity, pixmap);
  fz_run_display_list(ctx, data->getDisplayList(), dev, &ctm, &bbox, NULL);

  data->setImage(QImage(pixmap->samples, pixmap->w, pixmap->h, pixmap->stride,
                        QImage::Format_RGB888));
  data->setPixmap(pixmap);
  hashMutex.lock();
  dataHash.insert(pagenum, data);
  hashMutex.unlock();

  fz_close_device(ctx, dev);
  fz_drop_device(ctx, dev);
  fz_drop_context(ctx);

  //qDebug() << "Finished rendering:" << pagenum;
  emit obj->PageLoaded(pagenum);
}

void Renderer::renderPage(int pagenum, QSize DPI, int degrees) {
  //qDebug() << "- Rendering Page:" << pagenum << degrees;
  Data *data;
  fz_matrix matrix;
  fz_rect bbox;
  fz_display_list *list;

  double pageDPI = 96.0;
  double sf = DPI.width() / pageDPI;
  fz_scale(&matrix, sf, sf);
  fz_pre_rotate(&matrix, degrees);

  //mupdf indexes pages starting at 0, not 1
  pdf_page *PAGE = pdf_load_page(CTX, (pdf_document *)DOC, pagenum-1);
  pdf_bound_page(CTX, PAGE, &bbox);
  emit OrigSize(QSizeF(bbox.x1 - bbox.x0, bbox.y1 - bbox.y0));

  fz_transform_rect(&bbox, &matrix);
  list = fz_new_display_list(CTX, &bbox);
  fz_device *dev = fz_new_list_device(CTX, list);
  pdf_run_page_contents(CTX, PAGE, dev, &fz_identity, NULL);

  fz_link *link = pdf_load_links(CTX, PAGE);
  pdf_annot *_annot = pdf_first_annot(CTX, PAGE);
  QList<Annot *> annotList;
  QList<Widget *> widgetList;

  // qDebug() << "Starting annotations for:" << pagenum;
  while (_annot) {
    // if(pdf_annot_is_dirty(CTX, _annot))
    // qDebug() << "DIRTY ANNOT";

    int type = pdf_annot_type(CTX, _annot);
    /**TYPES
      0 = TEXT
      1 = LINK
      2 = FREE_TEXT
      3 = LINE
      4 = SQUARE
      ...
      24 = 3D
      -1 = UNKNOWN
    **/
    // qDebug() << "Page Number:" << pagenum+1 << type <<
    // pdf_string_from_annot_type(CTX, (enum pdf_annot_type)type);
    fz_rect anotBox;
    pdf_bound_annot(CTX, _annot, &anotBox);
    QRectF rect = convertRect(anotBox, sf);
    float opacity = pdf_annot_opacity(CTX, _annot);
    /**TYPES
      0 = LEFT
      1 = CENTER
      2 = RIGHT
    **/
    int quadding = pdf_annot_quadding(CTX, _annot);

    Annot *annot = new Annot(CTX, _annot, type, pagenum, opacity, rect);

    int flags = pdf_annot_flags(CTX, _annot);
    /**FLAGS
      1 << 0 = INVISIBLE
      1 << 1 = HIDDEN
      1 << 2 = PRINT
      1 << 3 = NO_ZOOM
      1 << 4 = NO_ROTATE
      1 << 5 = NO_VIEW
      1 << 6 = READ_ONLY
      1 << 7 = LOCKED
      1 << 8 = TOGGLE_NO_VIEW
      1 << 9 = LOCKED_CONTENTS
    **/

    annot->setPrint((flags & PDF_ANNOT_IS_PRINT) == PDF_ANNOT_IS_PRINT);

    char *contents = NULL, *author = NULL;
    if (type != 19 and type != 20) {
      fz_try(CTX) {
        contents = pdf_copy_annot_contents(CTX, _annot);
        if (contents)
          annot->setContents(QString::fromLocal8Bit(contents));
      }
      fz_catch(CTX) {}
      fz_try(CTX) {
        author = pdf_copy_annot_author(CTX, _annot);
        if (author)
          annot->setAuthor(QString::fromLocal8Bit(author));
      }
      fz_catch(CTX) {}
    }
    // pdf_annot_modification_date(CTX, _annot);

    if (pdf_annot_has_ink_list(CTX, _annot)) {
      int inkCount = pdf_annot_ink_list_count(CTX, _annot);
      QVector<QVector<QPointF>> inkList;
      for (int i = 0; i < inkCount; i++) {
        int strokeCount = pdf_annot_ink_list_stroke_count(CTX, _annot, i);
        QVector<QPointF> inkPoints;
        for (int k = 0; k < strokeCount; k++) {
          fz_point strokeVertex =
              pdf_annot_ink_list_stroke_vertex(CTX, _annot, i, k);
          QPointF vertexPoint(strokeVertex.x, strokeVertex.y);
          inkPoints.append(sf * vertexPoint);
        }
        inkList.append(inkPoints);
        annot->setInkList(inkList);
      }
      fz_rect annotRect;
      pdf_annot_rect(CTX, _annot, &annotRect);
      // qDebug() << "ANNOT RECT:" << convertRect(annotRect, sf);
      // qDebug() << "INK LIST:" << inkList;
    }

    float color[4] = {0, 0, 0, 1};
    int n;
    fz_try(CTX) {
      pdf_annot_color(CTX, _annot, &n, color);
      QColor inkColor = QColor(color[0] * 255, color[1] * 255, color[2] * 255,
                               color[3] * 255);
      annot->setColor(inkColor);
      // qDebug() << "COLOR:" << inkColor;
    }
    fz_catch(CTX) {
      // qDebug() << "NO COLOR";
      annot->setColor(QColor());
    }

    if (pdf_annot_has_interior_color(CTX, _annot)) {
      fz_try(CTX) {
        color[0] = 0;
        color[1] = 0;
        color[2] = 0;
        color[3] = 1;
        pdf_annot_interior_color(CTX, _annot, &n, color);
        QColor internalColor = QColor(color[0] * 255, color[1] * 255,
                                      color[2] * 255, color[3] * 255);
        // qDebug() << "INTERNAL COLOR:" << internalColor;
        annot->setInternalColor(internalColor);
      }
      fz_catch(CTX) {
        // qDebug() << "NO INTERNAL COLOR";
        annot->setInternalColor(QColor());
      }
    }

    // qDebug() << "BORDER:" << pdf_annot_border(CTX, _annot);

    if (pdf_annot_has_quad_points(CTX, _annot)) {
      // qDebug() << "HAS QUAD POINTS" << "Page Number:" << pagenum << type;
      int pointCount = pdf_annot_quad_point_count(CTX, _annot);
      QList<QPolygonF> quadList;
      for (int i = 0; i < pointCount; i++) {
        float qp[8];
        pdf_annot_quad_point(CTX, _annot, i, qp);
        QPolygonF quad =
            QPolygonF(QVector<QPointF>()
                      << QPointF(qp[0], qp[1]) << QPointF(qp[2], qp[3])
                      << QPointF(qp[4], qp[5]) << QPointF(qp[6], qp[7]));
        quadList.append(quad);
      }
      annot->setQuadList(quadList);
    }

    if (pdf_annot_has_line_ending_styles(CTX, _annot)) {
      // qDebug() << "HAS LINE ENDING STYLES" << "Page Number:" << pagenum <<
      // type;
      pdf_line_ending start, end;
      /**LINE ENDING
        0 = NONE
        1 = SQUARE
        2 = CIRCLE
        ...
        9 = SLASH
      **/
      pdf_annot_line_ending_styles(CTX, _annot, &start, &end);
      // qDebug() << "START:" << start << "END:" << end;
    }

    if (pdf_annot_has_vertices(CTX, _annot)) {
      // qDebug() << "HAS VERTICIES" << "Page Number:" << pagenum << type;
      int vertexCount = pdf_annot_vertex_count(CTX, _annot);
      QList<QPointF> vertexList;
      for (int i = 0; i < vertexCount; i++) {
        fz_point v = pdf_annot_vertex(CTX, _annot, i);
        vertexList.append(sf * QPointF(v.x, v.y));
      }
      // qDebug() << vertexList;
    }

    if (pdf_annot_has_line(CTX, _annot)) {
      fz_point a, b;
      pdf_annot_line(CTX, _annot, &a, &b);
      QPointF pa(a.x, a.y);
      QPointF pb(b.x, b.y);
      // qDebug() << pa << pb;
    }

    if (pdf_annot_has_icon_name(CTX, _annot)) {
      QString iconName =
          QString::fromLocal8Bit(pdf_annot_icon_name(CTX, _annot));
      // qDebug() << iconName;
    }

    if (!pdf_annot_has_ink_list(CTX, _annot)) {
      pdf_run_annot(CTX, _annot, dev, &fz_identity, NULL);
    }

    annotList.append(annot);
    _annot = pdf_next_annot(CTX, _annot);
  }

  pdf_widget *widget = pdf_first_widget(CTX, (pdf_document *)DOC, PAGE);
  while (widget) {
    int type = pdf_widget_type(CTX, widget);
    /**
      -1 = NOT_WIDGET
      0 = PUSHBUTTON
      1 = CHECKBOX
      2 = RADIOBUTTON
      3 = TEXT
      4 = LISTBOX
      5 = COMBOBOX
      6 = SIGNATURE
    **/

    fz_rect wrect;
    pdf_bound_widget(CTX, widget, &wrect);
    QRectF WRECT = convertRect(wrect, sf);
    char *currText = pdf_text_widget_text(CTX, (pdf_document *)DOC, widget);
    int maxLen = pdf_text_widget_max_len(CTX, (pdf_document *)DOC, widget);
    int contentType =
        pdf_text_widget_content_type(CTX, (pdf_document *)DOC, widget);
    /**
      0 = UNRESTRAINED
      1 = NUMBER
      2 = SPECIAL
      3 = DATE
      4 = TIME
    **/

    Widget *WIDGET = new Widget(type, WRECT, QString::fromLocal8Bit(currText),
                                maxLen, contentType);
    if (type == 4 or type == 5) {
      QStringList optionList, exportList;
      bool multi = pdf_choice_widget_is_multiselect(CTX, (pdf_document *)DOC, widget);

      if (int listS = pdf_choice_widget_options(CTX, (pdf_document *)DOC,
                                                widget, 0, NULL)) {
        char *opts[listS];
        pdf_choice_widget_options(CTX, (pdf_document *)DOC, widget, 0, opts);
        for (int i = 0; i < listS; i++) {
          optionList.append(QString::fromLocal8Bit(opts[i]));
        }
        WIDGET->setOptions(optionList);
      }

      if (int exportS = pdf_choice_widget_options(CTX, (pdf_document *)DOC,
                                                  widget, 1, NULL)) {
        char *opts[exportS];
        pdf_choice_widget_options(CTX, (pdf_document *)DOC, widget, 1, opts);
        for (int i = 0; i < exportS; i++) {
          exportList.append(QString::fromLocal8Bit(opts[i]));
        }
        WIDGET->setExports(exportList);
      }
    }

    widgetList.push_back(WIDGET);

    widget = pdf_next_widget(CTX, widget);
  }

  fz_close_device(CTX, dev);
  fz_drop_device(CTX, dev);
  // fz_drop_page(CTX, PAGE);

  data = new Data(pagenum, CTX, list, bbox, matrix, sf, link, annotList,
                  widgetList);
  data->setRenderThread(QtConcurrent::run(&renderer, data, this));
}

QList<TextData *> Renderer::searchDocument(QString text, bool matchCase) {
  //qDebug() << "Search Document";
  fz_rect rectBuffer[1000];
  QList<TextData *> results;
  for (int i = 0; i < pnum; i++) {
    int count =
        fz_search_display_list(CTX, dataHash[i]->getDisplayList(),
                               text.toLocal8Bit().data(), rectBuffer, 1000);
    // qDebug() << "Page " << i+1 << ": Count, " << count;
    for (int j = 0; j < count; j++) {
      TextData *t =
          new TextData(dataHash[i]->getScaledRect(rectBuffer[j]), i + 1, text);
      // MuPDF search does not match case, so retrieve the exact text at the
      // location found and determine whether or not it matches the case of the
      // search text if the user selected to match case
      if (matchCase) {
        fz_stext_page *sPage = fz_new_stext_page_from_display_list(
            CTX, dataHash[i]->getDisplayList(), NULL);
        QString currentStr =
            QString(fz_copy_selection(CTX, sPage, *fz_rect_min(&rectBuffer[j]),
                                      *fz_rect_max(&rectBuffer[j]), false));
        if (currentStr.contains(text, Qt::CaseSensitive)) {
          results.append(t);
        }
      } else {
        results.append(t);
      }
    }
  }
  return results;
}

QImage Renderer::imageHash(int pagenum) {
  if(!dataHash.contains(pagenum)){ return QImage(); }
  return dataHash[pagenum]->getImage();
}

QSize Renderer::imageSize(int pagenum){
  if(!dataHash.contains(pagenum)){ return QSize(); }
  return dataHash[pagenum]->getImage().size();
}

int Renderer::hashSize() { return dataHash.size(); }

void Renderer::clearHash(int pagenum) {
  //qDeleteAll(dataHash);
  if(pagenum<0){ dataHash.clear(); }
  else if(dataHash.contains(pagenum)){ dataHash.remove(pagenum); }
}

TextData *Renderer::linkList(int pagenum, int entry) {
  if(!dataHash.contains(pagenum)){ return 0; }
  return dataHash[pagenum]->getLinkList()[entry]->getData();
}

int Renderer::linkSize(int pagenum) {
  if(!dataHash.contains(pagenum)){ return 0; }
  return dataHash[pagenum]->getLinkList().size();
}

Annotation *Renderer::annotList(int pagenum, int entry) {
  if(!dataHash.contains(pagenum)){ return 0; }
  return static_cast<Annotation *>(dataHash[pagenum]->getAnnotList(entry));
}

int Renderer::annotSize(int pagenum) {
  if(!dataHash.contains(pagenum)){ return 0; }
  return dataHash[pagenum]->getAnnotSize();
}

Widget *Renderer::widgetList(int pagenum, int entry) {
  if(!dataHash.contains(pagenum)){ return 0; }
  return dataHash[pagenum]->getWidgetList(entry);
}

int Renderer::widgetSize(int pagenum) {
  if(!dataHash.contains(pagenum)){ return 0; }
  return dataHash[pagenum]->getWidgetSize();
}

bool Renderer::isDoneLoading(int page){ return dataHash.contains(page); }
bool Renderer::supportsExtraFeatures() { return true; }
