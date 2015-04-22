//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the main interface for any OS-specific system calls
//    To port Lumina to a different operating system, just create a file 
//    called "LuminaOS-<Operating System>.cpp", and use that file in 
//    the project (libLumina.pro) instead of LuminaOS-FreeBSD.cpp
//===========================================
#ifndef _LUMINA_LIBRARY_OS_H
#define _LUMINA_LIBRARY_OS_H

#include <QString>
#include <QStringList>
#include <QProcess>
#include <QDir>
#include <QObject>

#include "LuminaUtils.h"

//First try to load the globals.h to set PREFIX
#ifndef PREFIX
#include "global.h"
#endif
//Now use the default value for PREFIX if not set
#ifndef PREFIX
#define PREFIX QString("/usr/local")
#endif

class LOS{
public:
	//Return the name of the OS being used
	static QString OSName(); 

	//OS-specific prefix(s)
	static QString LuminaShare(){ return (PREFIX+"/share/Lumina-DE/"); } //Install dir for Lumina share files
	static QString AppPrefix(); //Prefix for applications (/usr/local/ on FreeBSD)
	static QString SysPrefix(); //Prefix for system (/usr/ on FreeBSD)

	//OS-specific application shortcuts (*.desktop files)
	static QString ControlPanelShortcut();
	static QString AppStoreShortcut();
	static QString QtConfigShortcut();

	//Scan for mounted external devices
	static QStringList ExternalDevicePaths(); //Returns: QStringList[<type>::::<filesystem>::::<path>]
	  //Note: <type> = [USB, HDRIVE, DVD, SDCARD, UNKNOWN]

	//Read screen brightness information
	static int ScreenBrightness(); //Returns: Screen Brightness as a percentage (0-100, with -1 for errors)
	//Set screen brightness
	static void setScreenBrightness(int percent);

	//Read the current volume
	static int audioVolume(); //Returns: audio volume as a percentage (0-100, with -1 for errors)
	//Set the current volume
	static void setAudioVolume(int percent);
	//Modify the current volume by a set amount (+ or -)
	static void changeAudioVolume(int percentdiff);

	//Check if a graphical audio mixer is installed
	static bool hasMixerUtility();
	//Launch the graphical audio mixer utility
	static void startMixerUtility();

	//Check for user system permission (shutdown/restart)
	static bool userHasShutdownAccess();
	//System Shutdown
	static void systemShutdown(); //start poweroff sequence
	//System Restart
	static void systemRestart(); //start reboot sequence
	//Check for suspend support
	static bool systemCanSuspend();
	//Put the system into the suspend state
	static void systemSuspend();


	//Battery Availability
	static bool hasBattery();
	//Battery Charge Level
	static int batteryCharge(); //Returns: percent charge (0-100), anything outside that range is counted as an error
	//Battery Charging State
	static bool batteryIsCharging();
	//Battery Time Remaining
	static int batterySecondsLeft(); //Returns: estimated number of seconds remaining
	
	//Get the checksum for a file
	static QStringList Checksums(QStringList filepaths); //Return: checksum of each input file (same order)
	
	//Get the filesystem capacity
	static QString FileSystemCapacity(QString dir) ; //Return: percentage capacity as give by the df command
};

#endif
