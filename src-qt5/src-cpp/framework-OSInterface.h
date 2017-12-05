//===========================================
//  Lumina desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the main interface for any OS-specific system calls
//    To port Lumina to a different operating system, just create a file
//    called "OSInterface-<Operating System>.cpp"
//===========================================
#ifndef _LUMINA_LIBRARY_OS_INTERFACE_H
#define _LUMINA_LIBRARY_OS_INTERFACE_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QObject>
#include <QVariant>
#include <QHash>

#include <QIODevice>
#include <QFileSystemWatcher>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>

class OSInterface : public QObject{
	Q_OBJECT
	// == QML ACCESSIBLE PROPERTIES ==
	//Battery
	Q_PROPERTY( float batteryCharge READ batteryCharge NOTIFY batteryChargeChanged)
	Q_PROPERTY( bool batteryCharging READ batteryCharging NOTIFY batteryChargingChanged)
	Q_PROPERTY( double batterySecondsLeft READ batterySecondsLeft NOTIFY batterySecondsLeftChanged)
	//Volume
	Q_PROPERTY( int volume READ volume WRITE setVolume NOTIFY volumeChanged)
	//Network
	Q_PROPERTY( bool networkAvailable READ networkAvailable NOTIFY networkStatusChanged)
	Q_PROPERTY( QString networkType READ networkType NOTIFY networkStatusChanged)
	Q_PROPERTY( float networkStrength READ networkStrength NOTIFY networkStatusChanged)
	Q_PROPERTY( QString networkHostname READ networkHostname NOTIFY networkStatusChanged)
	Q_PROPERTY( QHostAddress networkAddress READ networkAddress NOTIFY networkStatusChanged)
	//Media
	Q_PROPERTY( QStringList mediaShortcuts READ mediaShortcuts NOTIFY mediaShortcutsChanged)
	//Updates
	Q_PROPERTY( bool updatesAvailable READ updatesAvailable NOTIFY updateStatusChanged)
	Q_PROPERTY( bool updatesRunning READ updatesRunning NOTIFY updateStatusChanged)
	Q_PROPERTY( bool updatesFinished READ updatesFinished NOTIFY updateStatusChanged)
	//Power options
	Q_PROPERTY( bool canReboot READ canReboot NOTIFY powerAvailableChanged)
	Q_PROPERTY( bool canShutdown READ canShutdown NOTIFY powerAvailableChanged)
	Q_PROPERTY( bool canSuspend READ canSuspend NOTIFY powerAvailableChanged)
public:
	// ================
	// SEMI-VIRTUAL FUNCTIONS - NEED TO BE DEFINED IN THE OS-SPECIFIC FILES
	// ================
	//Start/stop interface watchers/notifications
	void start();
	void stop();
	bool isRunning(); //status of the object - whether it has been started yet

	// = Battery =
	Q_INVOKABLE bool batteryAvailable();
	Q_INVOKABLE float batteryCharge();
	Q_INVOKABLE bool batteryCharging();
	Q_INVOKABLE double batterySecondsLeft();
	// = Volume =
	Q_INVOKABLE bool volumeAvailable();
	Q_INVOKABLE int volume();
	Q_INVOKABLE void setVolume(int);
	// = Network Information =
	Q_INVOKABLE bool networkAvailable();
	Q_INVOKABLE QString networkType(); //"wifi", "wired", or "cell"
	Q_INVOKABLE float networkStrength(); //percentage. ("wired" type should always be 100%)
	Q_INVOKABLE QString networkHostname();
	Q_INVOKABLE QHostAddress networkAddress();
	// = Network Modification =

	// = Media Shortcuts =
	Q_INVOKABLE QStringList mediaDirectories(); //directory where XDG shortcuts are placed for interacting with media (local/remote)
	Q_INVOKABLE QStringList mediaShortcuts(); //List of currently-available XDG shortcut file paths
	// = Updates =
	Q_INVOKABLE bool updatesAvailable();
	Q_INVOKABLE QString updateDetails();	//Information about any available updates
	Q_INVOKABLE bool updatesRunning();
	Q_INVOKABLE QString updateLog();		//Information about any currently-running update
	Q_INVOKABLE bool updatesFinished();
	Q_INVOKABLE QString updateResults();	//Information about any finished update
	Q_INVOKABLE void startUpdates();
	Q_INVOKABLE bool updateOnlyOnReboot(); //Should the startUpdates function be called only when rebooting the system?
	Q_INVOKABLE QDateTime lastUpdate();	//The date/time of the previous updates
	Q_INVOKABLE QString lastUpdateResults(); //Information about the previously-finished update
	// = System Power =
	Q_INVOKABLE bool canReboot();
	Q_INVOKABLE void startReboot();
	Q_INVOKABLE bool canShutdown();
	Q_INVOKABLE void startShutdown();
	Q_INVOKABLE bool canSuspend();
	Q_INVOKABLE void startSuspend();

private slots:
	// ================
	// SEMI-VIRTUAL FUNCTIONS - NEED TO BE DEFINED IN THE OS-SPECIFIC FILES
	// ================
	//FileSystemWatcher slots
	void watcherFileChanged(QString);
	void watcherDirChanged(QString);
	//IO Device slots
	void iodeviceReadyRead();
	void iodeviceAboutToClose();
	//NetworkAccessManager slots
	void netAccessChanged(QNetworkAccessManager::NetworkAccessibility);
	void netRequestFinished(QNetworkReply*);
	void netSslErrors(QNetworkReply*, const QList<QSslError>&);

signals:
	void interfaceChanged(OSInterface::Interface);
	void batteryChargeChanged();
	void batteryChargingChanged();
	void batterySecondsLeftChanged();
	void volumeChanged();
	void networkStatusChanged();
	void mediaShortcutsChanged();
	void updateStatusChanged();
	void powerAvailableChanged();

private:
	//Internal persistant data storage, OS-specific usage implementation
	enum Interface{ Battery, Volume, Media, Network, PowerOff, Reboot, Suspend, Updates };
	QHash< OSInterface::Interface, QList<QVariant> > INFO;

	// ============
	// Internal possibilities for watching the system (OS-Specific usage/implementation)
	// ============
	//File System Watcher
	QFileSystemWatcher *watcher;
	//IO Device (QLocalSocket, QTcpConnection, QFile, etc)
	QIODevice *iodevice;
	//Network Access Manager (check network connectivity, etc)
	QNetworkAccessManager *netman;

	// Internal implifications for connecting the various watcher objects to their respective slots
	// (OS-agnostic - defined in the "OSInterface_private.cpp" file)
	void connectWatcher();
	void connectIodevice();
	void connectNetman();

public:
	OSInterface(QObject *parent = 0);
	~OSInterface();

	static OSInterface* instance(); //Get the currently-active instance of this class (or make a new one)

};
#endif
