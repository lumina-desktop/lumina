#ifndef LUMINA_PDF_LINK_H
#define LUMINA_PDF_LINK_H

namespace LuminaPDF {

class Link {
public:
  Link(TextData *_data, Poppler::Link *_link) : data(_data), link(_link) {}
  ~Link() { delete data; }

  TextData *getData() { return data; }
  Poppler::Link *getLink() { return link; }

private:
  TextData *data;
  Poppler::Link *link;
};

} // namespace LuminaPDF
#endif // LUMINA_PDF_LINK_H
