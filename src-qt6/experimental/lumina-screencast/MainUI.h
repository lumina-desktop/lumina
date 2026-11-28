//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================

#include <QMainWindow>

namespace Ui{
  class MainUI;
};

class MainUI : public QMainWindow {
  Q_Object
public:
  MainUI();
  ~MainUI();

public slots:
  void setupIcons();

private:
  Ui::MainUI *ui;

private slots:
  void closeApplication() {
    this->close();
  }
//protected:
//events go here
};
