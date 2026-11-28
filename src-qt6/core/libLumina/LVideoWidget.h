#ifndef LVIDEOWIDGET_H
#define LVIDEOWIDGET_H

#include <QtGlobal>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

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

#endif // QT_VERSION < 6.0.0
#endif // LVIDEOWIDGET_H
