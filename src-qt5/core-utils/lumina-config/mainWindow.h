//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_CONFIG_MAIN_WINDOW_H
#define _LUMINA_CONFIG_MAIN_WINDOW_H
#include "globals.h"

namespace Ui{
	class mainWindow;
};

class mainWindow : public QMainWindow{
	Q_OBJECT
public:
	mainWindow();
	~mainWindow();

public slots:
	void slotSingleInstance();
	void setupIcons();

private:
	Ui::mainWindow *ui;

private slots:

};
#endif
