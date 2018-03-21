//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#pragma once

#include <QDialog>
#include <QTextEdit>
#include "Renderer.h"

namespace Ui{
  class PropDialog;
};

class PropDialog : public QDialog {
  Q_OBJECT
  public:
    PropDialog(Renderer *Backend);

  public slots:
    void setSize(QSizeF);
    void setInformation();

  private:
    Ui::PropDialog *ui;
    Renderer *BACKEND;
};
