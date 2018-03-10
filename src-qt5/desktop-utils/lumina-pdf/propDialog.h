//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_PDF_VIEWER_PROP_DIALOG_H
#define _LUMINA_PDF_VIEWER_PROP_DIALOG_H

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

    void setSize(QSizeF);

  private:
    //void setInfo(fz_context*, pdf_obj*, QTextEdit*, QString);

    Ui::PropDialog *ui;
};
#endif
