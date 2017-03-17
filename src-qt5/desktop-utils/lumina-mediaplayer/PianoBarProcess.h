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
	void closePianoBar(); //"q"

	QString currentStation(); //Re-direct for the "autostartStation()" function;
	QStringList stations();
	void setCurrentStation(QString station);
	
	void deleteCurrentStation(); //"d"
	void createNewStation(); //"c"
	void createStationFromCurrentSong(); //"v"
	void changeStation(); //"s"

	//Settings Manipulation
	QString audioQuality(); 			// "audio_quality" = [low, medium, high]
	void setAudioQuality(QString); 	// [low, medium, high]
	QString autostartStation();		//"autostart_station" = ID 
	void setAutostartStation(QString);
	QString proxy();					//"proxy" = URL (example: "http://USER:PASSWORD@HOST:PORT/"  )
	void setProxy(QString);
	QString controlProxy();			//"control_proxy" = URL (example: "http://USER:PASSWORD@HOST:PORT/"  )
	void setControlProxy(QString);

public slots:
	void play(); // "P"
	void pause(); //"S" 

	void volumeDown(); //"("
	void volumeUp(); //")"

	void skipSong(); //"n"	
	void loveSong(); // "+"
	void tiredSong(); // "t"
	void banSong(); //"-"
	void bookmarkSong(); //"b"

	void explainSong(); //"e"

	void requestHistory(); // "h"
	void requestSongInfo(); //"i"
	void requestUpcoming(); //"u"


private:
	//Process
	QProcess PROC;
	void setupProcess();

	//Settings file management
	QStringList currentSettings; //cache of the settings file (file is really small);
	QString settingsPath; //location of the settings file
	void GenerateSettings();
	void loadSettings();
	QString settingValue(QString);
	void setSettingValue(QString,QString);
	void saveSettingsFile();

	//Cached Info
	QString cstation; //current station
        QStringList stationNames;

private slots:
	void ProcUpdate();
	

signals:
	void NewInformation(QString); //random status updates/information
	void NowPlayingStation(QString, QString); //[name, id]
	void NowPlayingSong(bool, QString,QString,QString, QString, QString); //[isLoved, title, artist, album, detailsURL, fromStation]
	void TimeUpdate(int, int); //[current secs, total secs];
};
#endif
