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

public: 
	enum Interface{ Battery, Volume, Devices, Network, PowerOff, Reboot, Suspend, Updates };

private slots:
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

private:
	//Internal persistant data storage, OS-specific usage implementation
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

	//Simplifications for connecting the various watcher objects to their respective slots
	void connectWatcher(){
	  if(watcher==0){ return; }
	  connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(watcherFileChanged(QString)) );
	  connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(watcherDirChanged(QString)) );
	}
	void connectIodevice(){
	  if(iodevice==0){ return; }
	  connect(iodevice, SIGNAL(readyRead()), this, SLOT(iodeviceReadyRead()) );
	}
	void connectNetman(){
	  if(netman==0){ return; }
	  connect(netman, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(netAccessChanged(QNetworkAccessManager::NetworkAccessibility)) );
	  connect(netman, SIGNAL(requestFinished(QNetworkReply*)), this, SLOT(netRequestFinished(QNetworkReply*)) );
	  connect(netman, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), this, SLOT(netSslErrors(QNetworkReply*, const QList<QSslError>&)) );
	}
	
public:
	OSInterface(QObject *parent = 0) : QObject(parent){
	  watcher = 0;
	  iodevice = 0;
	  netman = 0;
	}
	~OSInterface(){
	  if(watcher!=0){ 
	    QStringList paths; paths << watcher->files() << watcher->directories();
	    if(!paths.isEmpty()){ watcher->removePaths(paths); }
	    watcher->deleteLater();
	  }
	  if(iodevice!=0){
	    if(iodevice->isOpen()){ iodevice->close(); }
	    iodevice->deleteLater();
	  }
	  if(netman!=0){
	    netman->deleteLater();
	  }
	}

	//Simple functions used to determine if the current OS supports using this class, and what levels of support
	QList<OSInterface::Interface> supportedNotifications(); 	//Which interfaces provide change notifications
	QList<OSInterface::Interface> supportedStatus();		//Which interfaces are available for "status" requests
	QList<OSInterface::Interface> supportedModify();	//Which interfaces are available for "modify" requests

	//Start/stop interface watchers/notifications (each only called once per session)
	void start();
	void stop();
	
	//Generic status update
	QList<QVariant> status(OSInterface::Interface);
	// ==== Interface status output lists ====
	//  Battery: [ float (percent charge), bool (is Charging), double (seconds remaining) ];
	//  Volume: [int (percent volume) ]
	//  Devices: [ QStringList[ name, mountpoint, type (optional)] ]  (List length depends on number of devices)
	//  Network: [bool (network access available)]
	//  PowerOff: [bool (can power off system)]
	//  Reboot: [bool (can reboot system)]
	//  Suspend: [bool (can suspend system)]
	//  Updates: [bool (is updating), bool (reboot required)]
	// ==========

	//Individual Interface interactions
	bool modify(OSInterface::Interface, QList<QVariant> args); //returns: success/failure
	// ==== Interface modification argument lists ====
	//  Battery: <NO MODIFICATION>
	//  Volume: [int (set percent volume) ]
	//  Devices: <NO MODIFICATION>
	//  Network: <NO MODIFICATION>
	//  PowerOff: [bool (skip updates - optional)]
	//  Reboot: [bool (skip updates - optional)]
	//  Suspend: [] (No input arguments)
	//  Updates: <NO MODIFICATION>
	// ==========

signals:
	void interfaceChanged(OSInterface::Interface);

};
#endif
