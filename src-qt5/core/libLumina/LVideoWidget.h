#ifndef LVIDEOWIDGET_H
#define LVIDEOWIDGET_H

#include "LVideoLabel.h"
#include <QHBoxLayout>
#include <QResizeEvent>

class LVideoWidget : public QWidget {
  Q_OBJECT
  public:
    LVideoWidget(QString, QSize, bool icons, QWidget* parent=NULL);
    ~LVideoWidget();
    void setIconSize(QSize);
    void disableIcons();
    void enableIcons();

  private:
    LVideoLabel *iconLabel;
    QLabel *textLabel;
    QHBoxLayout *layout;
};
#endif
