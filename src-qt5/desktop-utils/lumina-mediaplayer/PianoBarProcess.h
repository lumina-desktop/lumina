//===========================================
//  Lumina-Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is a interface to the "pianobar" utility for streaming
//  audio from the Pandora radio service
//===========================================
#ifndef _LUMINA_PIANOBAR_PROCESS_H
#define _LUMINA_PIANOBAR_PROCESS_H

#include <QObject>
#include <QWidget>
#include <QProcess>
#include <QTimer>

// #define PIANOBAR_FIFO QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/pianobar/ctl"
// #define PIANOBAR_CONFIG QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/pianobar/config"

class PianoBarProcess : public QObject{
	Q_OBJECT
public:
	enum State {Stopped, Running, Paused};

	PianoBarProcess(QWidget *parent);
	~PianoBarProcess();

	State currentState();

	//Interaction functions
	bool isSetup(); //email/password already saved for use or not
	void setLogin(QString email, QString pass);
	QString email();
	QString password();

	void closePianoBar(); //"q"

	QString currentStation(); //Re-direct for the "autostartStation()" function;
	QStringList stations();
	void setCurrentStation(QString station);

	//Settings Manipulation
	QString audioQuality(); 			// "audio_quality" = [low, medium, high]
	void setAudioQuality(QString); 	// [low, medium, high]
	QString autostartStation();		//"autostart_station" = ID 
	void setAutostartStation(QString);
	QString proxy();					//"proxy" = URL (example: "http://USER:PASSWORD@HOST:PORT/"  )
	void setProxy(QString);
	QString controlProxy();			//"control_proxy" = URL (example: "http://USER:PASSWORD@HOST:PORT/"  )
	void setControlProxy(QString);

private:
	//Process
	QProcess *PROC;
	PianoBarProcess::State cState;
	QStringList infoList;
	void setupProcess();
	void sendToProcess(QString, bool withreturn = false);

	//Settings file management
	QStringList currentSettings; //cache of the settings file (file is really small);
	QString settingsPath; //location of the settings file
	QTimer *saveTimer;
	void GenerateSettings();
	bool loadSettings();
	QString settingValue(QString);
	void setSettingValue(QString,QString);


	//Cached Info
	QString cstation; //current station
	QStringList stationList;

public slots:
	void play(); // "P"
	void pause(); //"S" 

	void volumeDown(){ sendToProcess("("); } //"("
	void volumeUp(){ sendToProcess(")"); } //")"

	void skipSong(); //"n"	
	void loveSong(){ sendToProcess("+"); } // "+"
	void tiredSong(){ sendToProcess("t"); } // "t"
	void banSong(){ sendToProcess("-"); } //"-"
	void bookmarkSong(){ sendToProcess("b"); sendToProcess("s", true); } //"b"->"s"
	void bookmarkArtist(){ sendToProcess("b"); sendToProcess("a",true); } //"b"->"a"

	void deleteCurrentStation(); //"d"
	//void createNewStation(); //"c"
	void createStationFromCurrentSong(); //"v" -> "s"
	void createStationFromCurrentArtist(); //"v" -> "a"

	void explainSong(){ sendToProcess("e"); } //"e"

	void requestHistory(){ sendToProcess("h"); } // "h"  NOTE: Long series of interactive prompts - better to avoid for now
	void requestSongInfo(){ sendToProcess("i"); } //"i"  NOTE: This will re-print the current station/song information
	void requestUpcoming(){ sendToProcess("u"); } //"u" NOTE: This will often return "(i) No songs in queue" - best to run this after a "Receiving new playlist" info message

private slots:
	void ProcUpdate();
	void ProcStateChanged(QProcess::ProcessState);
	void saveSettingsFile();

signals:
	void NewInformation(QString); //random status updates/information
	void NowPlayingStation(QString, QString); //[name, id]
	void NowPlayingSong(bool, QString,QString,QString, QString, QString); //[isLoved, title, artist, album, detailsURL, fromStation]
	void TimeUpdate(int, int); //[current secs, total secs];
	void NewList(QStringList); //arranged in order: 0-end
	void StationListChanged(QStringList);
	void currentStateChanged(PianoBarProcess::State);
};
#endif
