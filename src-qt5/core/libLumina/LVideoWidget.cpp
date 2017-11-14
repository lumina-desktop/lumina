#include "LVideoWidget.h"

LVideoWidget::LVideoWidget(QString file, QSize iconSize, bool icons, QWidget *parent) : QWidget(parent) {
  iconLabel = new LVideoLabel(file, icons, parent);
  textLabel = new QLabel(parent);

  layout = new QHBoxLayout(this);
  layout->setAlignment(Qt::AlignLeft | Qt::AlignCenter);
  layout->setContentsMargins(5,5,5,5);
  layout->setStretchFactor(textLabel, 1); //make sure this always occupies all extra space

  textLabel->setText(file.section("/", -1));
  iconLabel->setGeometry(QRect(QPoint(0,0), iconSize));
  iconLabel->setFixedSize(iconSize);
  iconLabel->setVisible(true);
  textLabel->setVisible(true);

  layout->addWidget(iconLabel);
  layout->addWidget(textLabel);
}

LVideoWidget::~LVideoWidget() {
  delete iconLabel;
  delete textLabel;
  delete layout;
}

void LVideoWidget::setIconSize(QSize iconSize) {
  iconLabel->setFixedSize(iconSize);
}

void LVideoWidget::enableIcons() {
  iconLabel->enableIcons();
}

void LVideoWidget::disableIcons() {
  iconLabel->disableIcons();
}
