#ifndef LVIDEOWIDGET_H
#define LVIDEOWIDGET_H

#include "LVideoLabel.h"
#include <QHBoxLayout>
#include <QResizeEvent>

class LVideoWidget : public QWidget {
  Q_OBJECT
  public:
    LVideoWidget(QString, QSize, QWidget* parent=NULL);
    ~LVideoWidget();
    void setIconSize(QSize);

  private:
    bool flag;
    LVideoLabel *iconLabel;
    QLabel *textLabel;
    QHBoxLayout *layout;
};
#endif
