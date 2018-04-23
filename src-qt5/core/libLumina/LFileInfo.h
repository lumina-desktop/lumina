//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// File Information simplification class (combine QFileInfo with XDGDesktop)
//  Need some extra information not usually available by a QFileInfo
// ========================
#ifndef _LUMINA_LIBRARY_FILE_INFO_H
#define _LUMINA_LIBRARY_FILE_INFO_H

#include <LuminaXDG.h>
#include <QString>
#include <QFileInfo>
#include <QJsonObject>

class LFileInfo : public QFileInfo{
private:
	QString mime, zfs_ds, zfs_dspath;
	QStringList iconList, zfs_perms;
	int c_uid;
	XDGDesktop *desk;

	void loadExtraInfo();
	bool zfsAvailable();
	void getZfsDataset(); //note: only run this if "zfsAvailable" is true
	bool goodZfsDataset(); //simplification of the two functions above

public:
	//Couple overloaded contructors
	LFileInfo();
	LFileInfo(QString filepath);
	LFileInfo(QFileInfo info);
	~LFileInfo();

	//Functions for accessing the extra information
	// -- Return the mimetype for the file
	QString mimetype();

	// -- Return the icon file to use for this file
	QString iconfile(); //Note: This string is auto-formatted for use in the LXDG::findIcon() routine.

	// -- Check if this is an XDG desktop file
	bool isDesktopFile();

	// -- Allow access to the internal XDG desktop data structure
	XDGDesktop* XDG();

	//Other file type identification routines
	bool isImage(); //Is a readable image file (for thumbnail support)
	bool isVideo(); //Is a readable video file (for thumbnail support)
	bool isAVFile(); //Is an audio/video file

	//ZFS Information
	bool isZfsDataset(QString path = "");
	QString zfsPool();
	QStringList zfsSnapshots(); //Format: "snapshot name::::path/to/snapshot"
	QJsonObject zfsProperties();
	bool zfsSetProperty(QString property, QString value);

	//ZFS Permissions/Modifications
	bool canZFScreate();
	bool zfsCreateDataset(QString subdir);
	bool canZFSdestroy();
	bool zfsDestroyDataset(QString subdir);
	bool zfsDestroySnapshot(QString snapshot);
	bool canZFSclone();
	bool zfsCloneDataset(QString subdir, QString newsubdir);
	bool canZFSsnapshot();
	bool zfsSnapshot(QString snapname);
	bool canZFSrollback();
	bool zfsRollback(QString snapname);

};
typedef QList<LFileInfo> LFileInfoList;

#endif
