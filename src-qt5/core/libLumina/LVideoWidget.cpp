#include "LVideoWidget.h"

LVideoWidget::LVideoWidget(QString file, QSize iconSize, QWidget *parent) : QWidget(parent) {
  flag = false;
  iconLabel = new LVideoLabel(file, parent);
  textLabel = new QLabel(parent);

  layout = new QHBoxLayout(this);
  layout->setAlignment(Qt::AlignLeft | Qt::AlignCenter);
  layout->addWidget(iconLabel);
  layout->addWidget(textLabel);
  layout->setStretchFactor(textLabel, 1); //make sure this always occupies all extra space

  textLabel->setText(file.section("/", -1));
  iconLabel->setFixedSize(iconSize);
  iconLabel->setVisible(true);
  textLabel->setVisible(true);
}

LVideoWidget::~LVideoWidget() {
  delete iconLabel;
  delete textLabel;
  delete layout;
}

void LVideoWidget::setIconSize(QSize iconSize) {
  iconLabel->setFixedSize(iconSize);
}
