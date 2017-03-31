//===========================================
//  Lumina-Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_MEDIA_PLAYER_MAIN_UI_H
#define _LUMINA_MEDIA_PLAYER_MAIN_UI_H

#include <QMainWindow>
#include <QAction>
#include <QString>
#include <QStringList>

#include "PianoBarProcess.h"

namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

	void loadArguments(QStringList);

private:
	Ui::MainUI *ui;
	PianoBarProcess *PANDORA;

	void setupPandora();
	void setupConnections();

private slots:
	void PlayerTypeChanged();

	//Toolbar actions
	void playToggled();
	void pauseToggled();
	void stopToggled();
	void nextToggled();
	void backToggled();
	void volupToggled();
	void voldownToggled();

	//Pandora Options
	void showPandoraSongInfo();
	void changePandoraStation(QString);
	void applyPandoraSettings();
	//Pandora Process Feedback
	void PandoraStateChanged(PianoBarProcess::State);
	void NewPandoraInfo(QString);
	void PandoraStationChanged(QString);
	void PandoraSongChanged(bool, QString, QString, QString, QString, QString); //[isLoved, title, artist, album, detailsURL, fromStation]
	void PandoraTimeUpdate(int,int); //current secs, total secs
	void PandoraStationListChanged(QStringList);
	void PandoraListInfo(QStringList);
	
};
#endif
