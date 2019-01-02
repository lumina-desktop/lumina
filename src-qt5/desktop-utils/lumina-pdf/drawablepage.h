#ifndef LUMINA_PDF_DRAWABLEPAGE_H
#define LUMINA_PDF_DRAWABLEPAGE_H

#include <QImage>
#include <QSize>
#include <poppler/qt5/poppler-qt5.h>

namespace LuminaPDF {

class drawablePage {
public:
  drawablePage()
      : page(nullptr), DPI(0.0, 0.0), rotation(Poppler::Page::Rotate0) {}
  drawablePage(Poppler::Page *_page, QSize _DPI,
               Poppler::Page::Rotation _rotation)
      : page(_page), DPI(_DPI), rotation(_rotation) {}

  drawablePage(const LuminaPDF::drawablePage &other) = delete;
  LuminaPDF::drawablePage &
  operator=(const LuminaPDF::drawablePage &other) = delete;

  drawablePage(LuminaPDF::drawablePage &&other) = default;
  LuminaPDF::drawablePage &operator=(LuminaPDF::drawablePage &&other) = default;
  ~drawablePage() = default;

  QImage render() {
    return page->renderToImage(DPI.width(), DPI.height(), -1, -1, -1, -1,
                               rotation);
  }

  QSize size() { return page->pageSize(); }

private:
  std::unique_ptr<Poppler::Page> page;
  QSize DPI;
  Poppler::Page::Rotation rotation;
};

} // namespace LuminaPDF

#endif // LUMINA_PDF_DRAWABLEPAGE_H
