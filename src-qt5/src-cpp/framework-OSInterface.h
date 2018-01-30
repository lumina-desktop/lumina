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
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QVariant>

#include <QIODevice>
#include <QFileSystemWatcher>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>
#include <QHostInfo>
#include <QHostAddress>
#include <QNetworkConfiguration>
#include <QNetworkInterface>

//Lumina Utils class
#include <LUtils.h>

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
	//Brightness
	Q_PROPERTY( int brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)

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
	Q_INVOKABLE bool volumeSupported();
	Q_INVOKABLE int volume();
	Q_INVOKABLE void setVolume(int);

	// = Network Information =
	Q_INVOKABLE bool networkAvailable();
	Q_INVOKABLE QString networkType(); //"wifi", "wired", "cell", "cell-2G", "cell-3G", "cell-4G"
	Q_INVOKABLE QString networkTypeFromDeviceName(QString name); //wifi, wired, cell, cell-2G, cell-3G, cell-4G
	Q_INVOKABLE float networkStrength(); //percentage. ("wired" type should always be 100%)
	Q_INVOKABLE QString networkHostname();
	Q_INVOKABLE QHostAddress networkAddress();

	// = Network Modification =
	Q_INVOKABLE bool hasNetworkManager();
	Q_INVOKABLE QString networkManagerUtility(); //binary name or *.desktop filename (if registered on the system)

	// = Media Shortcuts =
	Q_INVOKABLE QStringList mediaDirectories(); //directory where XDG shortcuts are placed for interacting with media (local/remote)
	Q_INVOKABLE QStringList mediaShortcuts(); //List of currently-available XDG shortcut file paths

	// = Updates =
	Q_INVOKABLE bool updatesSupported(); //is thie subsystem supported for the OS?
	Q_INVOKABLE bool updatesAvailable();
	Q_INVOKABLE QString updateDetails();	//Information about any available updates
	Q_INVOKABLE bool updatesRunning();
	Q_INVOKABLE QString updateLog();		//Information about any currently-running update
	Q_INVOKABLE bool updatesFinished();
	Q_INVOKABLE QString updateResults();	//Information about any finished update
	Q_INVOKABLE void startUpdates();
	Q_INVOKABLE bool updateOnlyOnReboot(); //Should the startUpdates function be called only when rebooting the system?
	Q_INVOKABLE bool updateCausesReboot(); //Does the update power-cycle the system?
	Q_INVOKABLE QDateTime lastUpdate();	//The date/time of the previous updates
	Q_INVOKABLE QString lastUpdateResults(); //Information about the previously-finished update

	// = System Power =
	Q_INVOKABLE bool canReboot();
	Q_INVOKABLE void startReboot();
	Q_INVOKABLE bool canShutdown();
	Q_INVOKABLE void startShutdown();
	Q_INVOKABLE bool canSuspend();
	Q_INVOKABLE void startSuspend();

	// = Screen Brightness =
	Q_INVOKABLE bool brightnessSupported(); //is this subsystem available for the OS?
	Q_INVOKABLE int brightness(); //percentage: 0-100 with -1 for errors
	Q_INVOKABLE void setBrightness(int);

	// = System Status Monitoring
	Q_INVOKABLE bool cpuSupported(); //is this subsystem available for the OS?
	Q_INVOKABLE QList<int> cpuPercentage(); // (one per CPU) percentage: 0-100 with -1 for errors
	Q_INVOKABLE QStringList cpuTemperatures(); // (one per CPU) Temperature of CPU ("50C" for example)

	Q_INVOKABLE bool memorySupported(); //is this subsystem available for the OS?
	Q_INVOKABLE int memoryUsedPercentage(); //percentage: 0-100 with -1 for errors
	Q_INVOKABLE QString memoryTotal(); //human-readable form - does not tend to change within a session
	Q_INVOKABLE QStringList diskIO(); //Returns list of current read/write stats for each device

	Q_INVOKABLE bool diskSupported(); //is this subsystem available for the OS?
	Q_INVOKABLE int fileSystemPercentage(QString dir); //percentage of capacity used: 0-100 with -1 for errors
	Q_INVOKABLE QString fileSystemCapacity(QString dir); //human-readable form - total capacity

	// = OS-Specific Utilities =
	Q_INVOKABLE bool hasControlPanel();
	Q_INVOKABLE QString controlPanelShortcut(); //relative *.desktop shortcut name (Example: "some_utility.desktop")
	Q_INVOKABLE bool hasAudioMixer();
	Q_INVOKABLE QString audioMixerShortcut(); //relative *.desktop shortcut name (Example: "some_utility.desktop")
	Q_INVOKABLE bool hasAppStore();
	Q_INVOKABLE QString appStoreShortcut(); //relative *.desktop shortcut name (Example: "some_utility.desktop")


	// QML-Accessible properties/functions

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
	//Timer slots
	void BatteryTimerUpdate();
	void UpdateTimerUpdate();
	void BrightnessTimerUpdate();
	void VolumeTimerUpdate();
	void CpuTimerUpdate();
	void MemTimerUpdate();
	void DiskTimerUpdate();

signals:
	void batteryChargeChanged();
	void batteryChargingChanged();
	void batterySecondsLeftChanged();
	void volumeChanged();
	void networkStatusChanged();
	void mediaShortcutsChanged();
	void updateStatusChanged();
	void powerAvailableChanged();
	void brightnessChanged();

private:
	//Internal persistant data storage, OS-specific usage implementation
	QHash< QString, QVariant> INFO;
	bool _started;

	// ============
	// Internal possibilities for watching the system (OS-Specific usage/implementation)
	// ============
	//File System Watcher
	QFileSystemWatcher *watcher;
	//IO Device (QLocalSocket, QTcpConnection, QFile, etc)
	QIODevice *iodevice;
	//Network Access Manager (check network connectivity, etc)
	QNetworkAccessManager *netman;
	//Timer for regular probes/updates
	QTimer *batteryTimer, *updateTimer, *brightnessTimer, *volumeTimer, *cpuTimer, *memTimer, *diskTimer;

	// Internal implifications for connecting the various watcher objects to their respective slots
	// (OS-agnostic - defined in the "OSInterface_private.cpp" file)
	void connectWatcher(); //setup the internal connections *only*
	void connectIodevice(); //setup the internal connections *only*
	void connectNetman(); //setup the internal connections *only*

	//Internal simplification routines
	bool verifyAppOrBin(QString chk);

	// External Media Management (if system uses *.desktop shortcuts only)
	void setupMediaWatcher();
	bool handleMediaDirChange(QString dir); //returns true if directory was handled
	QStringList autoHandledMediaFiles();

	// Qt-based NetworkAccessManager usage
	void setupNetworkManager();

	// Timer-based monitors
	void setupBatteryMonitor(int update_ms, int delay_ms);
	void setupUpdateMonitor(int update_ms, int delay_ms);
	void setupBrightnessMonitor(int update_ms, int delay_ms);
	void setupVolumeMonitor(int update_ms, int delay_ms);
	void setupCpuMonitor(int update_ms, int delay_ms);
	void setupMemoryMonitor(int update_ms, int delay_ms);
	void setupDiskMonitor(int update_ms, int delay_ms);

	// Timer-based monitor update routines (NOTE: these are all run in a separate thread!!)
	void syncBatteryInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer);
	void syncUpdateInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer);
	void syncBrightnessInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer);
	void syncVolumeInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer);
	void syncCpuInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer);
	void syncMemoryInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer);
	void syncDiskInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer);

public:
	OSInterface(QObject *parent = 0);
	~OSInterface();

	static OSInterface* instance(); //Get the currently-active instance of this class (or make a new one)
	static void RegisterType(); //Register this object for QML access
};
#endif
