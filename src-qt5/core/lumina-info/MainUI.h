//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_INFORMATION_WINDOW_H
#define _LUMINA_INFORMATION_WINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QListWidgetItem>

namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

private:
	Ui::MainUI *ui;

	void updateUI();

private slots:
	void slotSingleInstance(){
	  this->show();
	  this->raise();
	}
	
	void showQtInfo();
	void showSponsor(QListWidgetItem *item);
	void LinkClicked(QString url);

	//Simplification slots for the link buttons
	void on_tool_bugs_clicked();
	void on_tool_irc_clicked();
	void on_tool_sources_clicked();
	void on_tool_website_clicked();
};
#endif
